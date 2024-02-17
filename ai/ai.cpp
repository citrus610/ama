#include "ai.h"

namespace AI
{

Result think_1p(Field field, Cell::Queue queue, Eval::Weight w, bool all_clear, i32 trigger)
{
    auto bsearch = Build::search(field, { queue[0], queue[1] }, w);
    auto asearch = Attack::search(field, queue);

    return AI::build(bsearch, asearch, all_clear, trigger);
};

Result build(Build::Result& bsearch, Attack::Result& asearch, bool all_clear, i32 trigger)
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
                .eval = 0,
            };
        }
    }

    i32 chain_score_max = 0;
    i32 attack_count = 0;

    for (auto& c : asearch.candidates) {
        chain_score_max = std::max(chain_score_max, c.attack_max.score);
        attack_count += c.attacks.size();
    }

    if (chain_score_max < trigger && !bsearch.candidates.empty()) {
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
        std::pair<i32, Result> best = { 0, Result() };

        for (auto& c : asearch.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            auto attack = c.attack_max;

            if (best.first < attack.score) {
                best.first = attack.score;
                best.second.placement = c.placement;
                best.second.plan = std::nullopt;
                best.second.eval = attack.score;
            }
        }

        return best.second;
    }

    return AI::RESULT_DEFAULT;
};

Result think_2p(Gaze::Player self, Gaze::Player enemy, Attack::Result& asearch, std::vector<Build::Result>& bsearch, Eval::Weight w[], i32 target_point)
{
    // Get attack balance
    i32 balance = self.attack - enemy.attack;

    // Gaze the enemy's field
    if (enemy.attack_frame > 0) {
        enemy.queue.erase(enemy.queue.begin());

        if (balance >= 5) {
            enemy.field.drop_garbage(balance);
        }
    }

    auto enemy_asearch = Attack::search(enemy.field, enemy.queue, false);
    auto enemy_gaze = Gaze::gaze(enemy.field, enemy_asearch, 4);

    bool enemy_small_field = Gaze::is_small_field(enemy.field, self.field);
    bool enemy_garbage_obstruct = Gaze::is_garbage_obstruct(enemy.field, Chain::Score { .count = enemy_gaze.main.count, .score = enemy_gaze.main.score });

    i32 enemy_harass_max = 0;
    i32 enemy_harass_fast_max = 0;

    for (auto& attack : enemy_gaze.harass) {
        enemy_harass_max = std::max(enemy_harass_max, attack.score / target_point);
    }

    for (auto& attack : enemy_gaze.harass_fast) {
        enemy_harass_fast_max = std::max(enemy_harass_fast_max, attack.score / target_point);
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
                            if (a.second.score_total == b.second.score_total) {
                                return a.second.frame_real > b.second.frame_real;
                            }
                            return a.second.score_total < b.second.score_total;
                        }
                    );
                }
            }

            if (best.second.count > 0) {
                // printf("rac\n");

                return Result {
                    .placement = best.first,
                    .plan = std::nullopt,
                    .eval = 0
                };
            }
        }

        i32 field_count = self.field.get_count();

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
                            return a.second.frame_real > b.second.frame_real;
                        }

                        if (a.second.score != b.second.score) {
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

                for (auto& attack : c.attacks_detect) {
                    find_best(c.placement, attack);
                }
            }

            if (best.second.score > 0) {
                if (enemy_attack >= 60 ||
                    enemy_small_field ||
                    enemy_garbage_obstruct) {
                    // printf("mf\n");
                    return Result {
                        .placement = best.first,
                        .plan = std::nullopt,
                        .eval = best.second.score
                    };
                }
            }
        }

        // Check if we can accept garbage
        i32 accept_limit = Gaze::get_accept_limit(self.field);

        if (enemy_attack <= accept_limit &&
            self.field.get_height(2) < 10 &&
            enemy_harass_fast_max <= 6) {
            i32 build_type = Build::Type::AC;

            if (enemy_attack <= 6) {
                build_type = Build::Type::BUILD;
            }

            if (bsearch.empty()) {
                auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[build_type]);
                return AI::build(b_result, asearch);
            }
            
            return AI::build(bsearch[build_type], asearch);
        }

        // Return attack if possible
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_syncro;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_main;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_small;
        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_desperate;

        auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
            if (attack.frame > enemy.attack_frame) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus_point) / target_point;

            if ((attack_send >= enemy_attack + 18 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 4) ||
                (attack_send >= enemy_attack + 12 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 3)) {
                attacks_syncro.push_back({ placement, attack });
            }

            if (attack_send >= enemy_attack) {
                if (attack.result.get_count() < std::max(24, field_count / 2)) {
                    attacks_main.push_back({ placement, attack });
                    return;
                }

                if (attack_send >= enemy_harass_fast_max + enemy_attack - 12 ||
                    attack.frame_real + attack.count * 2 <= enemy.attack_frame + 2) {
                    attack.redundancy = Gaze::get_redundancy(self.field, attack.result);
                    attacks_small.push_back({ placement, attack });
                }

                return;
            }

            if (attack_send >= enemy_attack - 6 &&
                attack.result.get_height(2) < 11 &&
                enemy_harass_fast_max < 6 &&
                !enemy_small_field &&
                !enemy_garbage_obstruct) {
                attack.redundancy = Gaze::get_redundancy(self.field, attack.result);
                attack.redundancy += enemy_attack - attack_send;
                attacks_small.push_back({ placement, attack });
                return;
            }

            if (attack_send + 30 < enemy_attack) {
                return;
            }

            attacks_desperate.push_back({ placement, attack });
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
            // If there is a fast and big attack then do it immediately
            // TLDR: Cross attack

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

            // printf("sync\n");

            return Result {
                .placement = best_syncro.first,
                .plan = std::nullopt,
                .eval = best_syncro.second.score
            };
        }

        if (!attacks_small.empty()) {
            // Return small attack

            auto best_small = *std::max_element(
                attacks_small.begin(),
                attacks_small.end(),
                [&] (const std::pair<Move::Placement, Attack::Data>& a, const std::pair<Move::Placement, Attack::Data>& b) {
                    i32 a_redundancy_over = a.second.redundancy > 6;
                    i32 b_redundancy_over = b.second.redundancy > 6;

                    if (a_redundancy_over != b_redundancy_over) {
                        return a_redundancy_over > b_redundancy_over;
                    }

                    i32 a_over_enemy_gaze = (a.second.score + self.bonus_point) / target_point >= enemy_attack + enemy_harass_fast_max;
                    i32 b_over_enemy_gaze = (b.second.score + self.bonus_point) / target_point >= enemy_attack + enemy_harass_fast_max;

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

                    if (a.second.count != b.second.count) {
                        return a.second.count > b.second.count;
                    }

                    if (a.second.score != b.second.score) {
                        return a.second.score < b.second.score;
                    }

                    return a.second.frame_real > b.second.frame_real;
                }
            );

            // printf("smol\n");

            return Result {
                .placement = best_small.first,
                .plan = std::nullopt,
                .eval = best_small.second.score
            };
        }

        if (!attacks_main.empty()) {
            // Return main chain

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

            // printf("m\n");

            return Result {
                .placement = best_main.first,
                .plan = std::nullopt,
                .eval = best_main.second.score
            };
        }

        if (!attacks_desperate.empty() && enemy.attack_frame <= 3) {
            // If we can't return the attack, try the biggest attack

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

            // printf("des\n");

            return Result {
                .placement = best_desperate.first,
                .plan = std::nullopt,
                .eval = best_desperate.second.score
            };
        }

        // If none possible, try to build fast

        auto build_type = Build::Type::SECOND_SMALL;

        if (enemy.attack_frame >= 16) {
            build_type = Build::Type::SECOND_BIG;
        }

        if (bsearch.empty()) {
            auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[build_type]);
            return AI::build(b_result, asearch, false);
        }
        
        return AI::build(bsearch[build_type], asearch, false);
    }

    // If the enemy is triggering a chain but it isn't big enough, then try to trigger an attack fast
    if (balance >= 0 && enemy.attack_frame > 0) {
        i32 enemy_height_2 = enemy.field.get_height(2);
        i32 enemy_field_count = enemy.field.get_count();

        std::vector<std::pair<Move::Placement, Attack::Data>> attacks_syncro;

        auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
            if (attack.frame_real + attack.count * 2 > enemy.attack_frame + 4) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus_point) / target_point;

            i32 attack_goal = 30;

            if (enemy_height_2 >= 5 && enemy_field_count >= 30) {
                attack_goal = 12;
            }

            if (attack_send >= attack_goal) {
                attack.redundancy = Gaze::get_redundancy(self.field, attack.result);
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

                    if (a.second.count != b.second.count) {
                        return a.second.count > b.second.count;
                    }

                    return a.second.frame > b.second.frame;
                }
            );

            // printf("sy hot\n");

            return Result {
                .placement = best.first,
                .plan = std::nullopt,
                .eval = best.second.score
            };
        }
    }

    // Build all clear
    if (enemy.all_clear) {
        if (bsearch.empty()) {
            auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[Build::Type::AC]);
            return AI::build(b_result, asearch);
        }
        
        return AI::build(bsearch[Build::Type::AC], asearch);
    }

    // Kill
    if (Gaze::is_garbage_obstruct(enemy.field, Chain::Score { .count = enemy_gaze.main.count, .score = enemy_gaze.main.score })) {
        i32 attack_need = (enemy_gaze.main.score / target_point) + 72 - enemy.field.get_mask().get_mask_12().get_count();

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

            // printf("kill\n");

            return Result {
                .placement = best.first,
                .plan = std::nullopt,
                .eval = best.second.score
            };
        }
    }

    // Harass
    if (self.field.get_count() >= 30 && self.field.get_count() < 54) {
        i32 attack_max = 0;

        for (auto& c : asearch.candidates) {
            if (c.attack_max.score > attack_max) {
                attack_max = c.attack_max.score;
            }
        }

        if (attack_max < 78000) {
            std::vector<std::pair<Move::Placement, Attack::Data>> attacks_harass;

            auto classify_attack = [&] (Move::Placement placement, Attack::Data& attack) {
                i32 attack_send = (attack.score + self.bonus_point) / target_point;

                if (attack.result.get_count() < 24) {
                    return;
                }

                if (attack.result.get_height(0) < 4 ||
                    attack.result.get_height(1) < 4 ||
                    attack.result.get_height(2) < 4) {
                    return;
                }

                if (attack_send < 6 || attack.count >= 4) {
                    return;
                }

                if (enemy_gaze.main_fast.score / target_point >= 30 && attack.count > 2) {
                    return;
                }

                attack.redundancy = Gaze::get_redundancy(self.field, attack.result);
                if (attack.redundancy >= 6) {
                    return;
                }

                // if (attack_send >= enemy_harass_max || attack_send >= enemy_gaze.main.score) {
                if (attack_send >= enemy_harass_max) {
                    attacks_harass.push_back({ placement, attack });
                }
            };

            for (auto& c : asearch.candidates) {
                for (auto& attack : c.attacks) {
                    classify_attack(c.placement, attack);
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
                        if (a.second.count != b.second.count) {
                            return a.second.count > b.second.count;
                        }

                        i32 a_enough = (a.second.score + self.bonus_point) / target_point >= enemy_harass_max + 6;
                        i32 b_enough = (b.second.score + self.bonus_point) / target_point >= enemy_harass_max + 6;

                        if (a_enough != b_enough) {
                            return a_enough < b_enough;
                        }

                        if (a_enough > 0 && b_enough > 0) {
                            return a.second.frame_real > b.second.frame_real;
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

            if (enemy_harass_max >= 12) {
                if (bsearch.empty()) {
                    auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[Build::Type::HARASS]);
                    return AI::build(b_result, asearch);
                }
            
                return AI::build(bsearch[Build::Type::HARASS], asearch);
            }
        }
    }

    // Build
    if (bsearch.empty()) {
        auto b_result = Build::search(self.field, { self.queue[0], self.queue[1] }, w[Build::Type::BUILD]);
        return AI::build(b_result, asearch);
    }
    
    return AI::build(bsearch[Build::Type::BUILD], asearch);
};

};