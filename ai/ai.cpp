#include "ai.h"

namespace AI
{

Result think_1p(
    Field field,
    Cell::Queue queue,
    Eval::Weight w,
    bool all_clear,
    i32 trigger,
    bool stretch
)
{
    auto bsearch = Build::search(field, { queue[0], queue[1] }, w);
    auto asearch = Attack::search(field, queue, false);

    return AI::build(bsearch, asearch, all_clear, trigger, stretch);
};

Result build(
    Build::Result& bsearch,
    Attack::Result& asearch,
    bool all_clear,
    i32 trigger,
    bool stretch
)
{
    if (bsearch.candidates.empty() && asearch.candidates.empty()) {
        return AI::RESULT_DEFAULT;
    }

    if (all_clear) {
        std::pair<Move::Placement, Attack::Data> best = { Move::Placement(), Attack::Data()};

        for (auto& c : asearch.candidates) {
            for (auto& attack : c.attacks_ac) {
                if (attack.frame > 4 || attack.count >= 4) {
                    continue;
                }

                best = std::max(
                    best,
                    { c.placement, attack },
                    [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                        if (a.second.score != b.second.score) {
                            return a.second.score < b.second.score;
                        }

                        return a.second.frame_real > b.second.frame_real;
                    }
                );
            }
        }

        if (best.second.count > 0) {
            return Result {
                .placement = best.first,
                .plan = std::nullopt,
                .eval = best.second.score,
            };
        }
    }

    i32 chain_score_max = 0;
    i32 attack_count = 0;

    for (auto& c : asearch.candidates) {
        chain_score_max = std::max(chain_score_max, c.attack_max.score);
        attack_count += c.attacks.size();
    }

    i32 q_max = 0;

    for (auto& c : bsearch.candidates) {
        q_max = std::max(q_max, c.eval.q);
    }

    bool trigger_condition = (chain_score_max > q_max && chain_score_max > 10000) || chain_score_max > trigger;

    if (!trigger_condition && !bsearch.candidates.empty()) {
        auto best = *std::max_element(
            bsearch.candidates.begin(),
            bsearch.candidates.end(),
            [&] (const Build::Candidate& a, const Build::Candidate& b) {
                if (a.eval.value != b.eval.value) {
                    return a.eval.value < b.eval.value;
                }

                return a.eval_fast < b.eval_fast;
            }
        );
        
        return Result {
            .placement = best.placement,
            .plan = best.eval.plan,
            .eval = best.eval.value,
        };
    }

    if (attack_count > 0) {
        std::pair<Attack::Data, Result> best = { Attack::Data(), Result() };

        for (auto& c : asearch.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            auto attack = c.attack_max;

            if (stretch) {
                if (Attack::cmp_main(best.first, attack)) {
                    best.first = attack;
                    best.second.placement = c.placement;
                    best.second.plan = std::nullopt;
                    best.second.eval = attack.score;
                }
            }
            else {
                if (Attack::cmp_main_enough(best.first, attack, trigger)) {
                    best.first = attack;
                    best.second.placement = c.placement;
                    best.second.plan = std::nullopt;
                    best.second.eval = attack.score;
                }
            }
        }

        return best.second;
    }

    return AI::RESULT_DEFAULT;
};


Result think_2p(
    Gaze::Player self,
    Gaze::Player enemy,
    Attack::Result& asearch,
    std::vector<Build::Result>& bsearch,
    Eval::Weight w[],
    i32 target_point,
    bool& form,
    Style::Data style,
    i32 trigger,
    bool stretch
)
{
    // Check field count and use forms
    i32 field_count = self.field.get_count();

    if (field_count >= 36 || self.field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() > 0) {
        form = false;
    }

    // Get attack balance
    i32 balance = self.attack - enemy.attack;

    // Speculate enemy's garbage dropping
    if (enemy.attack_frame > 0) {
        enemy.queue.erase(enemy.queue.begin());

        if (balance >= 3) {
            enemy.field.drop_garbage(balance);
        }
    }

    if (enemy.dropping > 0) {
        if (balance >= 3) {
            enemy.field.drop_garbage(balance);
        }
    }

    // Get attacks
    auto self_attacks = Attack::search(self.field, { self.queue[0], self.queue[1] });
    auto enemy_attacks = Attack::search(enemy.field, { enemy.queue[0], enemy.queue[1] });

    // Gaze enemy's field
    auto enemy_gaze = Gaze::gaze(enemy.field, enemy_attacks, enemy.attack_frame + (enemy.dropping > 0) * 2);

    bool enemy_small_field = Gaze::is_small_field(enemy.field, self.field);
    bool enemy_garbage_obstruct = Gaze::is_garbage_obstruct(enemy.field, Chain::Score { .score = std::max(enemy_gaze.main.score, enemy_gaze.main_fast.score ) });

    i32 enemy_harass_max = 0;
    i32 enemy_harass_fast_max = 0;
    i32 enemy_early_attack = 0;

    for (auto& attack : enemy_gaze.harass) {
        enemy_harass_max = std::max(enemy_harass_max, attack.score / target_point);

        if (attack.frame_real <= 4) {
            enemy_harass_fast_max = std::max(enemy_harass_fast_max, attack.score / target_point);
        }

        if (attack.result.get_count() <= 6 && attack.score / target_point >= 12 && attack.count <= 2) {
            enemy_early_attack = std::max(enemy_early_attack, attack.score / target_point);
        }
    }

    // If the enemy is attacking
    if (balance < 0) {
        i32 enemy_attack = std::abs(balance);

        // In case the enemy triggered an all-clear chain in the opening of the game
        if (enemy.all_clear && enemy.attack_chain <= 4) {
            std::pair<Move::Placement, Attack::Data> best = { Move::Placement(), Attack::Data()};

            for (auto& c : asearch.candidates) {
                for (auto& attack : c.attacks_ac) {
                    if (attack.frame > enemy.attack_frame) {
                        continue;
                    }

                    best = std::max(
                        best,
                        { c.placement, attack },
                        [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                            if (a.second.score == b.second.score) {
                                return a.second.frame_real > b.second.frame_real;
                            }

                            return a.second.score < b.second.score;
                        }
                    );
                }
            }

            if (best.second.count > 0) {
                return Result {
                    .placement = best.first,
                    .plan = std::nullopt,
                    .eval = best.second.score
                };
            }
        }

        // Check if we can trigger main chain right away
        {
            std::pair<Move::Placement, Attack::Data> best = { Move::Placement(), Attack::Data()};

            auto find_best = [&] (Move::Placement placement, Attack::Data& attack) {
                if (attack.frame > enemy.attack_frame) {
                    return;
                }

                i32 attack_send = (attack.score + self.bonus_point) / target_point;

                if (attack_send < enemy_attack) {
                    return;
                }

                if (attack.score < 2100) {
                    return;
                }

                best = std::max(
                    best,
                    { placement, attack },
                    [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                        i32 a_enough = (a.second.score + self.bonus_point) / target_point >= enemy_attack + 60;
                        i32 b_enough = (b.second.score + self.bonus_point) / target_point >= enemy_attack + 60;

                        if (a_enough != b_enough) {
                            return a_enough < b_enough;
                        }

                        if (a_enough > 0 && b_enough > 0) {
                            if (a.second.frame_real != b.second.frame_real) {
                                return a.second.frame_real > b.second.frame_real;
                            }

                            return a.second.score < b.second.score;
                        }

                        if (std::abs(a.second.score - b.second.score) / target_point > 6) {
                            return a.second.score < b.second.score;
                        }

                        return a.second.frame_real > b.second.frame_real;
                    }
                );
            };

            for (auto& c : asearch.candidates) {
                for (auto& attack : c.attacks) {
                    find_best(c.placement, attack);
                }
            }

            for (auto& c : self_attacks.candidates) {
                for (auto& attack : c.attacks_detect) {
                    find_best(c.placement, attack);
                }
            }

            if (best.second.score > 0) {
                if (best.second.score >= trigger ||
                    enemy_attack >= 90 ||
                    enemy_small_field ||
                    enemy_garbage_obstruct) {
                    printf("bf\n");

                    return Result {
                        .placement = best.first,
                        .plan = std::nullopt,
                        .eval = best.second.score
                    };
                }
            }
        }

        // Return attack if possible
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_syncro;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_main;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_small;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_desperate;

        // Classify return attacks
        auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
            if (attack.frame > enemy.attack_frame) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus_point) / target_point;

            // Check for syncro attacks
            if ((attack_send >= enemy_attack + 18 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 3) ||
                (attack_send >= enemy_attack + 12 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 2)) {
                attacks_syncro.push_back({ placement, attack });
            }

            if (attack_send >= enemy_attack) {
                // Return main chain
                if (attack.result.get_count() < std::max(24, field_count / 2)) {
                    attacks_main.push_back({ placement, attack });
                    return;
                }

                // Return small chain while checking possible enemy's harass
                if (attack_send >= enemy_harass_max + enemy_attack - 12 ||
                    attack.frame_real + attack.count * 2 <= enemy.attack_frame + 2) {
                    attack.redundancy = Gaze::get_redundancy(attack.parent, attack.result);
                    attacks_small.push_back({ placement, attack });
                }

                return;
            }

            // Check for small returns that
            if (attack_send >= enemy_attack - 6 &&
                attack.result.get_height(2) < 10 &&
                enemy_attack >= 12 &&
                enemy.attack_frame <= 4 &&
                enemy_harass_max < 6 &&
                !enemy_small_field &&
                !enemy_garbage_obstruct) {
                attack.redundancy = Gaze::get_redundancy(attack.parent, attack.result);
                attack.redundancy += enemy_attack - attack_send;
                attacks_small.push_back({ placement, attack });
                return;
            }

            if (attack_send + 30 < enemy_attack) {
                return;
            }

            if (attack_send == 0 && field_count <= 42) {
                return;
            }

            attacks_desperate.push_back({ placement, attack });
        };

        for (auto& c : asearch.candidates) {
            for (auto& attack : c.attacks) {
                classify_attack(c.placement, attack);
            }
        }

        for (auto& c : self_attacks.candidates) {
            for (auto& attack : c.attacks_detect) {
                classify_attack(c.placement, attack);
            }
        }

        // If there is a fast and big attack then do it immediately
        // TLDR: Cross attack
        if (!attacks_syncro.empty() && style.defense == Style::Defense::STRONG) {
            auto best_syncro = *std::max_element(
                attacks_syncro.begin(),
                attacks_syncro.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    if (a.second.score != b.second.score) {
                        return a.second.score < b.second.score;
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            printf("cross\n");

            return Result {
                .placement = best_syncro.first,
                .plan = std::nullopt,
                .eval = best_syncro.second.score
            };
        }

        // Check if we can accept garbage
        i32 accept_limit = Gaze::get_accept_limit(self.field);
        i32 resource_balance = Gaze::get_resource_balance(self.field, enemy.field);

        if (resource_balance <= -12) {
            accept_limit = std::max(accept_limit, (std::abs(resource_balance) / 6 + 1) * 6);
        }

        if (enemy_attack <= accept_limit &&
            self.field.get_height(2) < 10) {
            i32 build_type = Build::Type::AC;

            if (enemy_attack < 6 && enemy.attack_frame >= 4) {
                build_type = Build::Type::BUILD;
            }

            if (bsearch.empty()) {
                auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[build_type]);
                return AI::build(b_result, asearch, true, trigger, stretch);
            }
            
            return AI::build(bsearch[build_type], asearch, true, trigger, stretch);
        }

        // Return small attack
        if (!attacks_small.empty()) {
            auto best_small = *std::max_element(
                attacks_small.begin(),
                attacks_small.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    i32 a_redundancy_over = a.second.redundancy > 4;
                    i32 b_redundancy_over = b.second.redundancy > 4;

                    if (a_redundancy_over != b_redundancy_over) {
                        return a_redundancy_over > b_redundancy_over;
                    }

                    if (style.defense == Style::Defense::STRONG) {
                        i32 a_over_enemy_gaze = (a.second.score + self.bonus_point) / target_point >= enemy_attack + enemy_harass_max;
                        i32 b_over_enemy_gaze = (b.second.score + self.bonus_point) / target_point >= enemy_attack + enemy_harass_max;

                        if (a_over_enemy_gaze != a_over_enemy_gaze) {
                            return a_over_enemy_gaze < a_over_enemy_gaze;
                        }

                        i32 a_over_enemy = (a.second.score + self.bonus_point) / target_point > enemy_attack;
                        i32 b_over_enemy = (b.second.score + self.bonus_point) / target_point > enemy_attack;

                        if (a_over_enemy != b_over_enemy) {
                            return a_over_enemy < b_over_enemy;
                        }

                        if (a.second.all_clear != b.second.all_clear) {
                            return a.second.all_clear < b.second.all_clear;
                        }

                        if (a.second.frame_real + 2 * a.second.count != b.second.frame_real + 2 * b.second.count) {
                            return a.second.frame_real + 2 * a.second.count > b.second.frame_real + 2 * b.second.count;
                        }

                        if (a.second.score != b.second.score) {
                            return a.second.score < b.second.score;
                        }

                        if (a.second.link != b.second.link) {
                            return a.second.link < b.second.link;
                        }
                    }
                    else if (style.defense == Style::Defense::WEAK) {
                        if (a.second.all_clear != b.second.all_clear) {
                            return a.second.all_clear < b.second.all_clear;
                        }

                        i32 a_near = std::abs(((a.second.score + self.bonus_point) / target_point) - enemy_attack - 6);
                        i32 b_near = std::abs(((b.second.score + self.bonus_point) / target_point) - enemy_attack - 6);

                        if (a_near != b_near) {
                            return a_near > b_near;
                        }
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            printf("s\n");

            return Result {
                .placement = best_small.first,
                .plan = std::nullopt,
                .eval = best_small.second.score
            };
        }

        // TODO: check if we should receive or trigger the main chain
        // Trigger main chain if we have no choice because enemy has a double attack combo
        // Don't trigger but instead receive if the attack amount is small and we're not in danger

        // Return main chain
        if (!attacks_main.empty()) {
            auto best_main = *std::max_element(
                attacks_main.begin(),
                attacks_main.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    if (a.second.score != b.second.score) {
                        return a.second.score < b.second.score;
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            printf("b\n");

            return Result {
                .placement = best_main.first,
                .plan = std::nullopt,
                .eval = best_main.second.score
            };
        }

        // If we can't return the attack, try the biggest attack
        if (!attacks_desperate.empty() && enemy.attack_frame <= 3 && enemy_attack > std::min(accept_limit, 6)) {
            auto best_desperate = *std::max_element(
                attacks_desperate.begin(),
                attacks_desperate.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    if (a.second.score != b.second.score) {
                        return a.second.score < b.second.score;
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            printf("des\n");

            return Result {
                .placement = best_desperate.first,
                .plan = std::nullopt,
                .eval = best_desperate.second.score
            };
        }

        // If none possible, try to build fast
        auto build_type = Build::Type::FAST;

        if (enemy.attack_frame > 8) {
            build_type = Build::Type::FREESTYLE;
        }

        if (enemy_attack <= std::min(accept_limit, 6) && enemy.attack_frame <= 3) {
            build_type = Build::Type::AC;
        }

        i32 enough = INT32_MAX;

        if (enemy_attack >= 90) {
            enough = std::max(0, enemy_attack * target_point - self.bonus_point + 30 * target_point);
        }

        if (bsearch.empty()) {
            auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[build_type]);
            return AI::build(b_result, asearch, false, std::min(trigger, enough), stretch);
        }
        
        return AI::build(bsearch[build_type], asearch, false, std::min(trigger, enough), stretch);
    }

    // If the enemy is triggering a chain but it isn't big enough, then try to trigger an attack fast
    if (balance >= 0 && enemy.attack_frame > 0 && style.attack == Style::Attack::STRONG) {
        u8 enemy_heights[6];
        enemy.field.get_heights(enemy_heights);

        i32 enemy_height_dt = 0;
        for (auto i = 0; i < 5; ++i) {
            enemy_height_dt = std::max(enemy_height_dt, std::abs(i32(enemy_heights[i]) - i32(enemy_heights[i + 1])));
        }

        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_syncro;

        auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
            if (attack.frame_real + attack.count * 2 > enemy.attack_frame + 4) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus_point) / target_point;
            i32 attack_send_height = (attack_send / 6) + ((attack_send % 6) >= 3);

            if ((attack_send_height >= enemy_height_dt) || (attack_send_height + enemy_heights[2] >= 9)) {
                attack.redundancy = Gaze::get_redundancy(attack.parent, attack.result);
                attacks_syncro.push_back({ placement, attack });
            }
        };

        for (auto& c : asearch.candidates) {
            for (auto& attack : c.attacks) {
                classify_attack(c.placement, attack);
            }

            for (auto& attack : c.attacks_detect) {
                classify_attack(c.placement, attack);
            }
        }

        if (!attacks_syncro.empty()) {
            auto best = *std::max_element(
                attacks_syncro.begin(),
                attacks_syncro.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    i32 a_redundancy_over = a.second.redundancy > 6;
                    i32 b_redundancy_over = b.second.redundancy > 6;

                    if (a_redundancy_over != b_redundancy_over) {
                        return a_redundancy_over > b_redundancy_over;
                    }

                    if (a.second.all_clear != b.second.all_clear) {
                        return a.second.all_clear < b.second.all_clear;
                    }

                    if (a.second.frame + a.second.count * 2 != b.second.frame + b.second.count * 2) {
                        return a.second.frame_real + a.second.count * 2 > b.second.frame_real + b.second.count * 2;
                    }

                    if (a.second.score != b.second.score) {
                        return a.second.score < b.second.score;
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            return Result {
                .placement = best.first,
                .plan = std::nullopt,
                .eval = best.second.score
            };
        }
    }

    // Kill
    if (enemy_garbage_obstruct) {
        auto enemy_main = enemy_gaze.main;

        if (enemy_gaze.main_fast.score > enemy_main.score) {
            enemy_main = enemy_gaze.main_fast;
        }

        i32 attack_need = (enemy_main.score / target_point) + 72 - enemy.field.get_mask().get_mask_12().get_count() + enemy.all_clear * 30;

        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_kill;

        auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
            i32 attack_send = (attack.score + self.bonus_point) / target_point;

            if (attack_send >= attack_need) {
                attacks_kill.push_back({ placement, attack });
            }
        };

        for (auto& c : asearch.candidates) {
            for (auto& attack : c.attacks) {
                classify_attack(c.placement, attack);
            }

            for (auto& attack : c.attacks_detect) {
                classify_attack(c.placement, attack);
            }
        }

        if (!attacks_kill.empty()) {
            auto best = *std::max_element(
                attacks_kill.begin(),
                attacks_kill.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    if (a.second.all_clear != b.second.all_clear) {
                        return a.second.all_clear < b.second.all_clear;
                    }

                    if (a.second.frame_real != b.second.frame_real) {
                        return a.second.frame_real > b.second.frame_real;
                    }

                    return a.second.score < b.second.score;
                }
            );

            return Result {
                .placement = best.first,
                .plan = std::nullopt,
                .eval = best.second.score
            };
        }
    }

    // Harass
    // Don't harass if our resource is low
    if (((field_count >= 24 && field_count < 52) || (self.all_clear && !enemy.all_clear)) && style.attack != Style::Attack::NONE) {
        i32 attack_max = 0;

        for (auto& c : asearch.candidates) {
            if (c.attack_max.score > attack_max) {
                attack_max = c.attack_max.score;
            }
        }

        if (attack_max < trigger) {
            u8 enemy_heights[6];
            enemy.field.get_heights(enemy_heights);

            std::vector<std::pair<Move::Placement, Attack::Data>> attacks_harass;
            std::vector<std::pair<Move::Placement, Attack::Data>> attacks_prompt;

            auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
                // Remove attacks that happened after a long attack
                if ((attack.score_total - attack.score) / target_point > 4) {
                    return;
                }

                i32 attack_send = (attack.score_total + self.bonus_point) / target_point;
                i32 attack_send_height = (attack_send / 6) + ((attack_send % 6) >= 3);

                if (enemy_heights[2] + attack_send_height < 10) {
                    return;
                }

                i32 attack_result_count = attack.result.get_count();

                // Remove attacks that are too wasteful of resources
                if (attack_result_count < 24) {
                    return;
                }

                u8 heights[6];
                attack.result.get_heights(heights);

                if (heights[0] < 4 ||
                    heights[1] < 4 ||
                    heights[2] < 4 ||
                    heights[3] < 3 ||
                    heights[4] < 3 ||
                    heights[5] < 3) {
                    return;
                }

                // Remove weak & long attacks
                if (attack_send < 6 || attack.count >= 3) {
                    return;
                }

                // Remove attacks that change the field a lot
                attack.redundancy = Gaze::get_redundancy(attack.parent, attack.result);
                if (attack.redundancy > 4) {
                    return;
                }

                if (style.attack == Style::Attack::STRONG) {
                    if (attack.count == 2 && attack.score / target_point < 12) {
                        return;
                    }

                    if (attack.count == 3 && attack.score / target_point <= 20) {
                        return;
                    }

                    if (attack.count == 1 && (attack_send - 6 >= enemy_gaze.defence_1dub.score / target_point)) {
                        attacks_harass.push_back({ placement, attack });
                        return;
                    }

                    if (attack.count == 2 && (attack_send - 12 >= enemy_gaze.defence_2dub.score / target_point)) {
                        attacks_harass.push_back({ placement, attack });
                        return;
                    }

                    if (attack.count == 3 && (attack_send - 12 >= enemy_gaze.defence_3dub.score / target_point)) {
                        attacks_harass.push_back({ placement, attack });
                        return;
                    }
                }
                else if (style.attack == Style::Attack::WEAK) {
                    if (attack_send >= std::min(8, enemy_harass_fast_max) && attack_send <= 18) {
                        attacks_harass.push_back({ placement, attack });
                    }
                }
            };

            auto classify_prompt = [&] (Move::Placement placement, Attack::Data& attack) {
                if (attack.count > 1 || attack.score != attack.score_total) {
                    return;
                }

                i32 attack_send = (attack.score_total + self.bonus_point) / target_point;

                i32 attack_result_count = attack.result.get_count();

                // Remove attacks that are too wasteful of resources
                if (attack_result_count < 30) {
                    return;
                }

                u8 heights[6];
                attack.result.get_heights(heights);

                if (heights[0] < 4 ||
                    heights[1] < 4 ||
                    heights[2] < 4 ||
                    heights[3] < 4 ||
                    heights[4] < 4 ||
                    heights[5] < 4) {
                    return;
                }

                // Remove strong & long attacks
                if (attack_send >= 6) {
                    return;
                }

                // Remove attacks that change the field a lot
                attack.redundancy = Gaze::get_redundancy(attack.parent, attack.result);
                if (attack.redundancy > 2) {
                    return;
                }

                attacks_prompt.push_back({ placement, attack });
            };

            for (auto& c : asearch.candidates) {
                for (auto& attack : c.attacks) {
                    classify_attack(c.placement, attack);
                    // classify_prompt(c.placement, attack);
                }

                // for (auto& attack : c.attacks_detect) {
                //     classify_attack(c.placement, attack);
                // }
            }

            if (!attacks_harass.empty()) {
                auto best = *std::max_element(
                    attacks_harass.begin(),
                    attacks_harass.end(),
                    [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                        i32 a_enough = (a.second.score + self.bonus_point) / target_point >= enemy_harass_fast_max + 6;
                        i32 b_enough = (b.second.score + self.bonus_point) / target_point >= enemy_harass_fast_max + 6;

                        if (a_enough != b_enough) {
                            return a_enough < b_enough;
                        }

                        if (a.second.frame + a.second.count * 2 != b.second.frame + b.second.count * 2) {
                            return a.second.frame + a.second.count * 2 > b.second.frame + b.second.count * 2;
                        }

                        if (a.second.link != b.second.link) {
                            return a.second.link < b.second.link;
                        }

                        return a.second.frame_real > b.second.frame_real;
                    }
                );

                return Result {
                    .placement = best.first,
                    .plan = std::nullopt,
                    .eval = best.second.score
                };
            }
            // If we can't crush, the prompt the enemy
            // else if (
            //     !attacks_prompt.empty() &&
            //     Gaze::get_resource_balance(self.field, enemy.field) >= -9) {
            //     auto best = *std::max_element(
            //         attacks_prompt.begin(),
            //         attacks_prompt.end(),
            //         [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
            //             if (a.second.redundancy != b.second.redundancy) {
            //                 return a.second.redundancy > b.second.redundancy;
            //             }

            //             if (a.second.score != b.second.score) {
            //                 return a.second.score < b.second.score;
            //             }

            //             return a.second.frame_real > b.second.frame_real;
            //         }
            //     );

            //     return Result {
            //         .placement = best.first,
            //         .plan = std::nullopt,
            //         .eval = best.second.score
            //     };
            // }

            // if (enemy_harass_fast_max >= 18) {
            //     if (bsearch.empty()) {
            //         auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[Build::Type::HARASS]);
            //         return AI::build(b_result, asearch, true, trigger, stretch);
            //     }
            
            //     return AI::build(bsearch[Build::Type::HARASS], asearch, true, trigger, stretch);
            // }
        }
    }

    auto build_type = Build::Type::BUILD;

    if (!form) {
        build_type = Build::Type::FREESTYLE;
    }

    if (enemy_garbage_obstruct) {
        build_type = Build::Type::FAST;
        form = false;
    }

    // Build fast if our resource is low
    // if (Gaze::is_small_field(self.field, enemy.field) ||
    //     (enemy_early_attack >= 18 && field_count < 30)) {
    //     build_type = Build::Type::AC;
    //     form = false;
    // }
    if (Gaze::is_small_field(self.field, enemy.field)) {
        build_type = Build::Type::AC;
        form = false;
    }

    // Build all clear
    if (enemy.all_clear) {
        build_type = Build::Type::AC;
        form = false;
    }

    // Build
    if (bsearch.empty()) {
        auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[build_type]);
        return AI::build(b_result, asearch, true, trigger, stretch);
    }
    
    return AI::build(bsearch[build_type], asearch, true, trigger, stretch);
};

};