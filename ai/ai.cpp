#include "ai.h"

namespace ai
{

Result build(
    Field field,
    cell::Queue queue,
    search::Result bsearch,
    search::Configs configs,
    dfs::attack::Result& asearch,
    search::Type type,
    i32 trigger,
    bool all_clear,
    bool stretch
)
{
    // Triggers all clear
    if (all_clear) {
        std::pair<move::Placement, dfs::attack::Data> best = { move::Placement(), dfs::attack::Data()};

        for (auto& c : asearch.candidates) {
            for (auto& attack : c.attacks_ac) {
                if (attack.frame > 4 || attack.count >= 4) {
                    continue;
                }

                best = std::max(
                    best,
                    { c.placement, attack },
                    [] (const std::pair<move::Placement, dfs::attack::Data>& a, const std::pair<move::Placement, dfs::attack::Data>& b) {
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
                .eval = best.second.score,
                .update = Update()
            };
        }
    }

    // Builds with beam search if possible
    if (type == search::Type::BUILD) {
        if (!bsearch.build.candidates.empty()) {
            return ai::Result {
                .placement = bsearch.build.candidates.front().placement,
                .eval = i32(bsearch.build.candidates.front().score) / 6,
                .update = Update()
            };
        }
        
        type = search::Type::FREESTYLE;
    }

    // Builds using dfs
    dfs::build::Result dfs_build;

    switch (type)
    {
    case search::Type::FREESTYLE:
        dfs_build = bsearch.freestyle;
        break;
    case search::Type::FAST:
        dfs_build = bsearch.fast;
        break;
    case search::Type::AC:
        dfs_build = bsearch.ac;
        break;
    }

    // Checks valid
    if (dfs_build.candidates.empty()) {
        dfs::eval::Weight dfs_w;

        switch (type)
        {
        case search::Type::FREESTYLE:
            dfs_w = configs.freestyle;
            break;
        case search::Type::FAST:
            dfs_w = configs.fast;
            break;
        case search::Type::AC:
            dfs_w = configs.ac;
            break;
        }

        dfs_build = dfs::build::search(field, { queue[0], queue[1] }, dfs_w);
    }

    if (dfs_build.candidates.empty() && asearch.candidates.empty()) {
        return ai::RESULT_DEFAULT;
    }

    // Builds
    i32 chain_score_max = 0;
    i32 attack_count = 0;

    for (auto& c : asearch.candidates) {
        chain_score_max = std::max(chain_score_max, c.attack_max.score);
        attack_count += c.attacks.size();
    }

    i32 q_max = 0;

    for (auto& c : dfs_build.candidates) {
        q_max = std::max(q_max, c.eval.q);
    }

    bool trigger_condition = (chain_score_max > q_max && chain_score_max > 10000) || chain_score_max > trigger;

    if (!trigger_condition && !dfs_build.candidates.empty()) {
        auto best = *std::max_element(
            dfs_build.candidates.begin(),
            dfs_build.candidates.end(),
            [&] (const dfs::build::Candidate& a, const dfs::build::Candidate& b) {
                if (a.eval.value != b.eval.value) {
                    return a.eval.value < b.eval.value;
                }

                return a.eval_fast < b.eval_fast;
            }
        );
        
        return Result {
            .placement = best.placement,
            .eval = best.eval.value,
            .update = Update()
        };
    }

    if (attack_count > 0) {
        std::pair<dfs::attack::Data, Result> best = { dfs::attack::Data(), Result() };

        for (auto& c : asearch.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            auto attack = c.attack_max;

            if (stretch) {
                if (dfs::attack::cmp_main(best.first, attack)) {
                    best.first = attack;
                    best.second.placement = c.placement;
                    best.second.eval = attack.score;
                }
            }
            else {
                if (dfs::attack::cmp_main_enough(best.first, attack, trigger)) {
                    best.first = attack;
                    best.second.placement = c.placement;
                    best.second.eval = attack.score;
                }
            }
        }

        return best.second;
    }

    return ai::RESULT_DEFAULT;
};

Result think(
    gaze::Player self,
    gaze::Player enemy,
    search::Result bsearch,
    search::Configs configs,
    i32 target_point,
    style::Data style,
    i32 trigger,
    bool stretch
)
{
    // Checks field count
    i32 field_count = self.field.get_count();

    // Gets attack balance
    i32 balance = self.attack - enemy.attack;

    // Speculates enemy's garbage dropping
    if (enemy.attack_frame > 0 && balance >= 3) {
        enemy.field.drop_garbage(balance);
    }

    if (enemy.dropping >= 3) {
        enemy.field.drop_garbage(balance);
    }

    // Gets attacks
    auto self_attacks = dfs::attack::search(self.field, self.queue);
    auto enemy_attacks = dfs::attack::search(enemy.field, { enemy.queue[0], enemy.queue[1] });

    // Reads enemy's field
    i32 enemy_delay = enemy.attack_frame + (enemy.dropping > 0) + (enemy.attack_frame > 0 && balance > 0);

    auto enemy_gaze = gaze::gaze(enemy.field, enemy_attacks, enemy_delay);

    bool enemy_small_field = gaze::is_small_field(enemy.field, self.field);
    bool enemy_garbage_obstruct = gaze::is_garbage_obstruct(enemy.field, chain::Score { .count = enemy_gaze.main.count, .score = enemy_gaze.main.score });

    i32 enemy_harass = enemy_gaze.harass.score / target_point;
    i32 enemy_early_attack = enemy_gaze.early.score / target_point;

    // If the enemy is attacking
    if (balance < 0) {
        i32 enemy_attack = std::abs(balance);

        // Step 1: In case the enemy triggered an all-clear chain in the opening of the game
        if (enemy.all_clear && enemy.attack_chain <= 4) {
            auto best = Action();

            for (auto& c : self_attacks.candidates) {
                for (auto& attack : c.attacks_ac) {
                    if (attack.frame > enemy.attack_frame) {
                        continue;
                    }

                    best = std::max(
                        best,
                        { c.placement, attack },
                        [&] (const Action& a, const Action& b) {
                            if (a.attack.score == b.attack.score) {
                                return a.attack.frame_real > b.attack.frame_real;
                            }

                            return a.attack.score < b.attack.score;
                        }
                    );
                }
            }

            if (best.attack.count > 0) {
                return Result {
                    .placement = best.placement,
                    .eval = best.attack.score,
                    .update = Update()
                };
            }
        }

        // Step 2: Checks if we can trigger main chain right away
        {
            auto best = Action();

            auto find_best = [&] (const move::Placement& placement, const dfs::attack::Data& attack) {
                if (attack.frame > enemy.attack_frame) {
                    return;
                }

                i32 attack_send = (attack.score + self.bonus) / target_point;

                if (attack_send < enemy_attack) {
                    return;
                }

                if (attack.score < 2100) {
                    return;
                }

                best = std::max(
                    best,
                    { placement, attack },
                    [&] (const Action& a, const Action& b) {
                        i32 a_enough = (a.attack.score + self.bonus) / target_point >= enemy_attack + 90;
                        i32 b_enough = (b.attack.score + self.bonus) / target_point >= enemy_attack + 90;

                        if (a_enough != b_enough) {
                            return a_enough < b_enough;
                        }

                        if (a_enough > 0 && b_enough > 0) {
                            if (a.attack.frame_real != b.attack.frame_real) {
                                return a.attack.frame_real > b.attack.frame_real;
                            }

                            return a.attack.score < b.attack.score;
                        }

                        if (std::abs(a.attack.score - b.attack.score) / target_point >= 6) {
                            return a.attack.score < b.attack.score;
                        }

                        return a.attack.frame_real > b.attack.frame_real;
                    }
                );
            };

            for (auto& c : self_attacks.candidates) {
                for (auto& attack : c.attacks) {
                    find_best(c.placement, attack);
                }

                for (auto& attack : c.attacks_detect) {
                    find_best(c.placement, attack);
                }
            }

            if (best.attack.score > 0) {
                if (best.attack.score >= trigger ||
                    enemy_attack >= 90 ||
                    enemy.attack_frame >= 10 ||
                    enemy_small_field ||
                    enemy_garbage_obstruct) {
                    return Result {
                        .placement = best.placement,
                        .eval = best.attack.score,
                        .update = Update()
                    };
                }
            }
        }

        // Step 3: Classifies return attack types
        // Returns attack if possible
        std::vector<Action> attacks_syncro;
        std::vector<Action> attacks_main;
        std::vector<Action> attacks_small;
        std::vector<Action> attacks_desperate;

        // Classify return attacks
        auto classify_attack = [&] (const move::Placement& placement, dfs::attack::Data& attack) {
            if (attack.frame > enemy.attack_frame) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus) / target_point;

            // Check for syncro attacks
            if ((attack_send >= enemy_attack + 24 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 4) ||
                (attack_send >= enemy_attack + 18 && attack.frame_real + attack.count * 2 <= enemy.attack_frame + 3) ||
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
                if (attack_send >= enemy_harass + enemy_attack - 12 ||
                    attack.frame_real + attack.count * 2 <= enemy.attack_frame + 2) {
                    attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);
                    attacks_small.push_back({ placement, attack });
                }

                return;
            }

            // Checks for small returns that are not sufficient
            if (attack_send >= enemy_attack - 6 &&
                attack.result.get_height(2) < 10 &&
                enemy_attack >= 12 &&
                enemy.attack_frame <= 4 &&
                enemy_harass < 6 &&
                !enemy_small_field &&
                !enemy_garbage_obstruct) {
                attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);
                attack.redundancy += enemy_attack - attack_send;
                attacks_small.push_back({ placement, attack });
                return;
            }

            // Checks for desparate returns
            if (attack_send + 30 < enemy_attack) {
                return;
            }

            if (attack_send == 0 && field_count <= 42) {
                return;
            }

            attacks_desperate.push_back({ placement, attack });
        };

        // Filters attacks
        for (auto& c : self_attacks.candidates) {
            for (auto& attack : c.attacks) {
                classify_attack(c.placement, attack);
            }

            for (auto& attack : c.attacks_detect) {
                classify_attack(c.placement, attack);
            }
        }

        // Step 4: If there is a fast and big attack then do it immediately
        // TLDR: Cross attack
        if (!attacks_syncro.empty() && style.defense == style::Defense::STRONG) {
            auto best_syncro = *std::max_element(
                attacks_syncro.begin(),
                attacks_syncro.end(),
                [&] (const Action& a, const Action& b) {
                    if (a.attack.score != b.attack.score) {
                        return a.attack.score < b.attack.score;
                    }

                    return a.attack.frame_real > b.attack.frame_real;
                }
            );

            return Result {
                .placement = best_syncro.placement,
                .eval = best_syncro.attack.score,
                .update = Update()
            };
        }

        // Step 5: Checks if we can accept garbage
        // Gets the amount of garbages that we can accept
        i32 accept_limit = gaze::get_accept_limit(self.field);
        i32 resource_balance = gaze::get_resource_balance(self.field, enemy.field);

        if (resource_balance <= -12) {
            accept_limit = std::max(accept_limit, (std::abs(resource_balance) / 6 + 1) * 6);
        }

        // Gets the frame limit for accepting garbage
        // If there aren't many garbages, we shouldn't accept those if they drop in a long time
        // We should only accept small garbages if the remaining time is small
        i32 accept_frame = 18;

        if (enemy_attack < 6) {
            accept_frame = 4;
        }

        // Accepts
        if (enemy_attack <= accept_limit &&
            enemy.attack_frame < accept_frame) {
            // TODO: creates another dfs for stalling and counter

            return ai::build(
                self.field,
                self.queue,
                bsearch,
                configs,
                self_attacks,
                search::Type::AC,
                trigger,
                true,
                stretch
            );
        }

        // Step 6: Returns small attack
        if (!attacks_small.empty()) {
            auto best_small = *std::max_element(
                attacks_small.begin(),
                attacks_small.end(),
                [&] (const Action& a, const Action& b) {
                    // Compares attacks' redundancy, the less the better 
                    // Obviously, we want a stable board, not a broken mess!
                    i32 a_redundancy_over = a.attack.redundancy > 4;
                    i32 b_redundancy_over = b.attack.redundancy > 4;

                    if (a_redundancy_over != b_redundancy_over) {
                        return a_redundancy_over > b_redundancy_over;
                    }

                    // If the defense style is strong
                    if (style.defense == style::Defense::STRONG) {
                        // Checks if any of these attacks are bigger than the incomming attack and the possible next attack from the enemy
                        // Basically, anti combo measure
                        i32 a_over_enemy_gaze = (a.attack.score + self.bonus) / target_point >= enemy_attack + enemy_harass;
                        i32 b_over_enemy_gaze = (b.attack.score + self.bonus) / target_point >= enemy_attack + enemy_harass;

                        if (a_over_enemy_gaze != a_over_enemy_gaze) {
                            return a_over_enemy_gaze < a_over_enemy_gaze;
                        }

                        // Checks if these attacks return garbages
                        i32 a_over_enemy = (a.attack.score + self.bonus) / target_point > enemy_attack;
                        i32 b_over_enemy = (b.attack.score + self.bonus) / target_point > enemy_attack;

                        if (a_over_enemy != b_over_enemy) {
                            return a_over_enemy < b_over_enemy;
                        }

                        // Simple compare function
                        if (a.attack.all_clear != b.attack.all_clear) {
                            return a.attack.all_clear < b.attack.all_clear;
                        }

                        if (a.attack.frame_real + 2 * a.attack.count != b.attack.frame_real + 2 * b.attack.count) {
                            return a.attack.frame_real + 2 * a.attack.count > b.attack.frame_real + 2 * b.attack.count;
                        }

                        if (a.attack.score != b.attack.score) {
                            return a.attack.score < b.attack.score;
                        }

                        if (a.attack.link != b.attack.link) {
                            return a.attack.link < b.attack.link;
                        }
                    }
                    // If the defense style is weak
                    else if (style.defense == style::Defense::WEAK) {
                        if (a.attack.all_clear != b.attack.all_clear) {
                            return a.attack.all_clear < b.attack.all_clear;
                        }

                        i32 a_near = std::abs(((a.attack.score + self.bonus) / target_point) - enemy_attack - 6);
                        i32 b_near = std::abs(((b.attack.score + self.bonus) / target_point) - enemy_attack - 6);

                        if (a_near != b_near) {
                            return a_near > b_near;
                        }
                    }

                    return a.attack.frame_real > b.attack.frame_real;
                }
            );

            // printf("s\n");

            return Result {
                .placement = best_small.placement,
                .eval = best_small.attack.score,
                .update = Update()
            };
        }

        // Step 7: Returns main chain
        // We only do this if:
        // - There are no other choice
        // - The enemy has a big attack next for a combo
        if (!attacks_main.empty()) {
            auto best_main = *std::max_element(
                attacks_main.begin(),
                attacks_main.end(),
                [&] (const Action& a, const Action& b) {
                    if (std::abs(a.attack.score - b.attack.score) / target_point >= 6) {
                        return a.attack.score < b.attack.score;
                    }

                    return a.attack.frame_real > b.attack.frame_real;
                }
            );

            // printf("b\n");

            return Result {
                .placement = best_main.placement,
                .eval = best_main.attack.score,
                .update = Update()
            };
        }

        // Step 8: Desperate return
        // If we can't return the attack, try the biggest attack possible
        if (!attacks_desperate.empty() && enemy.attack_frame <= 3 && enemy_attack > std::min(accept_limit, 6)) {
            auto best_desperate = *std::max_element(
                attacks_desperate.begin(),
                attacks_desperate.end(),
                [&] (const Action& a, const Action& b) {
                    if (a.attack.score != b.attack.score) {
                        return a.attack.score < b.attack.score;
                    }

                    return a.attack.frame_real > b.attack.frame_real;
                }
            );

            // printf("des\n");

            return Result {
                .placement = best_desperate.placement,
                .eval = best_desperate.attack.score,
                .update = Update()
            };
        }

        // Step 9: If none possible, try to build
        auto build_type = search::Type::FAST;

        if (enemy.attack_frame > 10) {
            build_type = search::Type::FREESTYLE;
        }

        if (enemy_attack <= std::min(accept_limit, 6) && enemy.attack_frame <= 3) {
            build_type = search::Type::AC;
        }

        // TODO:
        // return trigger update
        if (enemy_attack >= 50000 / target_point) {
            build_type = search::Type::BUILD;
        }

        if (enemy_attack >= 20000 / target_point && field_count >= 36) {
            build_type = search::Type::BUILD;
        }

        i32 enough = INT32_MAX;

        if (enemy_attack >= 90) {
            enough = (enemy_attack + 60) * target_point;
        }

        auto ai_build = ai::build(
            self.field,
            self.queue,
            bsearch,
            configs,
            self_attacks,
            build_type,
            std::min(trigger, enough),
            false,
            stretch
        );

        ai_build.update = Update {
            .form = false,
            .trigger = std::min(trigger, enough)
        };

        return ai_build;
    }

    // If the enemy is triggering a chain but it isn't big enough, then try to trigger an attack fast
    if (balance >= 0 && enemy.attack_frame > 0 && style.attack == style::Attack::STRONG) {
        u8 enemy_heights[6];
        enemy.field.get_heights(enemy_heights);

        std::vector<Action> attacks_syncro;

        // Classifies syncro attacks
        auto classify_attack = [&] (const move::Placement& placement, dfs::attack::Data& attack) {
            if (attack.frame_real + attack.count * 2 > enemy.attack_frame + 4) {
                return;
            }

            i32 attack_send = (attack.score + self.bonus) / target_point;
            i32 attack_goal = std::min(12, (11 - i32(enemy_heights[2])) * 6);

            if (attack_send >= attack_goal) {
                attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);
                attacks_syncro.push_back({ placement, attack });
            }
        };

        // Filters
        for (auto& c : self_attacks.candidates) {
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
                [&] (const Action& a, const Action& b) {
                    i32 a_redundancy_over = a.attack.redundancy > 6;
                    i32 b_redundancy_over = b.attack.redundancy > 6;

                    if (a_redundancy_over != b_redundancy_over) {
                        return a_redundancy_over > b_redundancy_over;
                    }

                    if (a.attack.all_clear != b.attack.all_clear) {
                        return a.attack.all_clear < b.attack.all_clear;
                    }

                    if (a.attack.frame + a.attack.count * 2 != b.attack.frame + b.attack.count * 2) {
                        return a.attack.frame_real + a.attack.count * 2 > b.attack.frame_real + b.attack.count * 2;
                    }

                    if (a.attack.score != b.attack.score) {
                        return a.attack.score < b.attack.score;
                    }

                    return a.attack.frame_real > b.attack.frame_real;
                }
            );

            return Result {
                .placement = best.placement,
                .eval = best.attack.score,
                .update = Update()
            };
        }
    }

    // Kill
    if (enemy_garbage_obstruct) {
        // Calculates the amount of attacks needed
        i32 enemy_main = std::max(enemy_gaze.main.score, enemy_gaze.main_q.score) / target_point;

        u8 enemy_heights[6];
        enemy.field.get_heights(enemy_heights);

        i32 enemy_height_min = *std::min_element(enemy_heights, enemy_heights + 6);

        i32 attack_need = enemy_main + (12 - enemy_height_min) * 6 + enemy.all_clear * 30;

        // Finds killing move
        std::vector<Action> attacks_kill;

        auto classify_attack = [&] (const move::Placement& placement, dfs::attack::Data& attack) {
            i32 attack_send = (attack.score + self.bonus) / target_point;

            if (attack_send >= attack_need) {
                attacks_kill.push_back({ placement, attack });
            }
        };

        for (auto& c : self_attacks.candidates) {
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
                [&] (const Action& a, const Action& b) {
                    if (a.attack.all_clear != b.attack.all_clear) {
                        return a.attack.all_clear < b.attack.all_clear;
                    }

                    if (a.attack.frame_real != b.attack.frame_real) {
                        return a.attack.frame_real > b.attack.frame_real;
                    }

                    return a.attack.score < b.attack.score;
                }
            );

            return Result {
                .placement = best.placement,
                .eval = best.attack.score,
                .update = Update()
            };
        }
    }

    // Harassment
    bool field_side_enough =
        self.field.get_height(0) > 3 &&
        self.field.get_height(1) > 3 &&
        self.field.get_height(2) > 3;

    if (((field_side_enough && field_count >= 24 && field_count < 52) || self.all_clear) && style.attack != style::Attack::NONE) {
        i32 attack_max = 0;

        for (auto& c : self_attacks.candidates) {
            if (c.attack_max.score > attack_max) {
                attack_max = c.attack_max.score;
            }
        }

        if (attack_max < trigger) {
            u8 enemy_heights[6];
            enemy.field.get_heights(enemy_heights);

            std::vector<Action> attacks_crush;
            std::vector<Action> attacks_combo;

            // Finds crushing moves
            auto classify_crush = [&] (const move::Placement& placement, dfs::attack::Data& attack) {
                // Removes attacks that happened after a long attack
                if ((attack.score_total - attack.score) / target_point > 4) {
                    return;
                }

                i32 attack_send = (attack.score_total + self.bonus) / target_point;
                i32 attack_send_height = (attack_send / 6) + ((attack_send % 6) >= 3);
                i32 attack_result_count = attack.result.get_count();

                // Removes insufficient attacks
                if (!self.all_clear && (enemy_heights[2] + attack_send_height <= 8)) {
                    return;
                }

                // Remove attacks that are too wasteful of resources
                u8 heights[6];
                attack.result.get_heights(heights);

                if (!self.all_clear) {
                    if (attack_result_count < 24) {
                        return;
                    }

                    if (heights[0] < 4 ||
                        heights[1] < 4 ||
                        heights[2] < 4 ||
                        heights[3] < 3 ||
                        heights[4] < 3 ||
                        heights[5] < 3) {
                        return;
                    }
                }
                else if (enemy.all_clear) {
                    if (attack_result_count < 24) {
                        return;
                    }
                }

                // Remove weak & long attacks
                if (attack_send < 6 || attack.count > 2) {
                    return;
                }

                // Remove attacks that change the field a lot
                attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);

                if (attack.redundancy > 4) {
                    return;
                }

                if (style.attack == style::Attack::STRONG) {
                    if (attack.count == 2 && attack.score / target_point < 12) {
                        return;
                    }

                    if (attack.count == 3 && attack.score / target_point <= 20) {
                        return;
                    }

                    if (attack.count == 1 && (attack_send >= (enemy_gaze.defence_1.score + enemy.bonus) / target_point)) {
                        attacks_crush.push_back({ placement, attack });
                        return;
                    }

                    if (attack.count == 2 && (attack_send >= (enemy_gaze.defence_2.score + enemy.bonus) / target_point)) {
                        attacks_crush.push_back({ placement, attack });
                        return;
                    }

                    if (attack.count == 3 && (attack_send >= (enemy_gaze.defence_2.score + enemy.bonus) / target_point)) {
                        attacks_crush.push_back({ placement, attack });
                        return;
                    }
                }
                else if (style.attack == style::Attack::WEAK) {
                    if (attack_send >= std::min(8, enemy_harass) && attack_send <= 18) {
                        attacks_crush.push_back({ placement, attack });
                    }
                }
            };

            // Finds combo move
            auto classify_combo = [&] (const move::Placement& placement, dfs::attack::Data& attack) {
                // Removes too long to reach attacks
                if (attack.frame > 6) {
                    return;
                }

                // Calculates send
                i32 attack_send = (attack.score_total + self.bonus) / target_point;
                i32 attack_send_prompt = (attack.score_total - attack.score + self.bonus) / target_point;
                i32 attack_send_combo = attack.score / target_point;

                // Removes attacks without prompt or sufficient combo
                if (attack_send_prompt < 4 || attack_send_prompt > 16) {
                    return;
                }

                if (attack_send_combo < 30) {
                    return;
                }

                // Removes attacks that are too wasteful of resources
                u8 heights[6];
                attack.result.get_heights(heights);

                if (heights[0] < 3 ||
                    heights[1] < 3 ||
                    heights[2] < 3 ||
                    heights[3] < 3 ||
                    heights[4] < 3 ||
                    heights[5] < 3) {
                    return;
                }

                // Removes attacks that change the field a lot
                attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);

                if (attack.redundancy > 6) {
                    return;
                }

                attacks_combo.push_back({ placement, attack });
            };

            // Filter
            bool combo_side_enough =
                self.field.get_height(0) > 4 &&
                self.field.get_height(1) > 4 &&
                self.field.get_height(2) > 4;

            for (auto& c : self_attacks.candidates) {
                for (auto& attack : c.attacks) {
                    classify_crush(c.placement, attack);
                    classify_combo(c.placement, attack);
                }

                if (combo_side_enough) {
                    for (auto& attack : c.attacks_detect) {
                        classify_combo(c.placement, attack);
                    }
                }
            }

            // Crush attacks
            if (!attacks_crush.empty()) {
                auto best = *std::max_element(
                    attacks_crush.begin(),
                    attacks_crush.end(),
                    [&] (const Action& a, const Action& b) {
                        i32 a_enough = (a.attack.score + self.bonus) / target_point >= enemy_harass + 6;
                        i32 b_enough = (b.attack.score + self.bonus) / target_point >= enemy_harass + 6;

                        if (a_enough != b_enough) {
                            return a_enough < b_enough;
                        }

                        i32 a_combo = (a.attack.score_total - a.attack.score + self.bonus) / target_point > 0;
                        i32 b_combo = (b.attack.score_total - b.attack.score + self.bonus) / target_point > 0;

                        if (a_combo != b_combo) {
                            return a_combo < b_combo;
                        }

                        if (a.attack.frame + a.attack.count * 2 != b.attack.frame + b.attack.count * 2) {
                            return a.attack.frame + a.attack.count * 2 > b.attack.frame + b.attack.count * 2;
                        }

                        if (a.attack.link != b.attack.link) {
                            return a.attack.link < b.attack.link;
                        }

                        return a.attack.frame_real > b.attack.frame_real;
                    }
                );

                return Result {
                    .placement = best.placement,
                    .eval = best.attack.score,
                    .update = Update()
                };
            }

            // Combo attacks
            if (!attacks_combo.empty()) {
                auto best = *std::max_element(
                    attacks_combo.begin(),
                    attacks_combo.end(),
                    [&] (const Action& a, const Action& b) {
                        if (a.attack.frame != b.attack.frame * 2) {
                            return a.attack.frame > b.attack.frame;
                        }

                        if (a.attack.link != b.attack.link) {
                            return a.attack.link < b.attack.link;
                        }

                        return a.attack.frame_real > b.attack.frame_real;
                    }
                );

                return Result {
                    .placement = best.placement,
                    .eval = best.attack.score,
                    .update = Update()
                };
            }
        }
    }

    // Builds
    auto build_type = search::Type::BUILD;
    bool form = true;

    if (enemy_garbage_obstruct) {
        build_type = search::Type::FAST;
        form = false;
    }

    // Build fast if our resource is low
    if (gaze::is_small_field(self.field, enemy.field)) {
        build_type = search::Type::AC;
        form = false;
    }

    // Build all clear
    if (enemy.all_clear) {
        build_type = search::Type::AC;
        form = false;
    }

    // Build
    auto ai_build = ai::build(
        self.field,
        self.queue,
        bsearch,
        configs,
        self_attacks,
        build_type,
        trigger,
        true,
        stretch
    );

    // if (!form) {
    //     ai_build.update.form = false;
    // }

    return ai_build;
};

};