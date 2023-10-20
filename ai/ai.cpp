#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 trigger)
{
    auto search = Search::search(field, queue);

    if (search.candidates.empty()) {
        return AI::RESULT_DEFAULT;
    }

    AI::get_candidate_eval(search, w);
    return AI::build(search, field, trigger);
};

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Weight w)
{
    auto search_result = Search::search(field, queue);

    if (search_result.candidates.empty()) {
        return AI::RESULT_DEFAULT;
    }

    if (enemy.attack < 0 && enemy.attack_frame > 0) {
        enemy.field.drop_garbage(std::abs(enemy.attack));
    }

    // Gaze
    Chain::Score enemy_detect_highest;
    Chain::Score enemy_detect_harass;
    AI::get_gaze_field(enemy.field, enemy.queue, enemy_detect_highest, enemy_detect_harass);

    // Enemy attack
    if (enemy.attack > 0) {
        // Try all clear
        if (enemy.all_clear) {
            std::pair<Move::Placement, Search::Attack> best = { Move::Placement(), Search::Attack()};

            for (auto& c : search_result.candidates) {
                for (auto& attack : c.attacks) {
                    if (!attack.all_clear || attack.frame > enemy.attack_frame) {
                        continue;
                    }

                    best = std::max(
                        best,
                        { c.placement, attack },
                        [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                            if (a.second.score_total == b.second.score_total) {
                                return a.second.frame_total > b.second.frame_total;
                            }
                            return a.second.score_total < b.second.score_total;
                        }
                    );
                }
            }

            if (best.second.count > 0) {
                return Result {
                    .placement = best.first,
                    .eval = 0,
                };
            }
        }

        // Enemy haste early
        if (field.get_count() <= 30 &&
            enemy.attack < 18 &&
            !AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
            printf("recs\n");
            AI::get_candidate_eval(search_result, w.allclear);
            return AI::build(search_result, field, 70000);
        }

        // If enemy is in danger, or had triggered their biggest chain, or our chain is big enough
        {
            std::pair<Move::Placement, Search::Attack> best = { Move::Placement(), Search::Attack()};

            for (auto& c : search_result.candidates) {
                for (auto& attack : c.attacks) {
                    if (attack.frame > enemy.attack_frame) {
                        continue;
                    }

                    best = std::max(
                        best,
                        { c.placement, attack },
                        [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                            if (a.second.score == b.second.score) {
                                return a.second.frame > b.second.frame;
                            }
                            return a.second.score < b.second.score;
                        }
                    );
                }
            }

            if (best.second.score >= 78000) {
                printf("rebig\n");
                return Result {
                    .placement = best.first,
                    .eval = best.second.score
                };
            }

            if ((best.second.score_total + data.bonus) / data.target >= enemy.attack) {
                if (AI::get_small_field(enemy.field, field) ||
                    AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
                    printf("rebig\n");
                    return Result {
                        .placement = best.first,
                        .eval = best.second.score
                    };
                }
            }
        }

        // Receive
        if (!data.all_clear &&
            field.get_height(2) < 10 &&
            field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() <= 6 &&
            field.get_count() <= 48 &&
            enemy.attack <= 4 &&
            !AI::get_small_field(enemy.field, field) &&
            !AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
            printf("recb\n");
            AI::get_candidate_eval(search_result, w.build);
            return AI::build(search_result, field, 78000);
        }

        // Check returnable
        bool returnable = false;

        for (auto& c : search_result.candidates) {
            for (auto& attack : c.attacks) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                if ((attack.score_total + data.bonus) / data.target >= enemy.attack) {
                    returnable = true;
                    break;
                }
            }

            if (returnable) {
                break;
            }
        }

        // Last ditch
        if (!returnable) {
            if (enemy.attack_frame <= 3) {
                AI::iterate_candidates(search_result, [&] (Search::Candidate& c) {
                    for (auto& attack : c.attacks) {
                        if (attack.frame > enemy.attack_frame) {
                            continue;
                        }

                        i32 attack_count = (attack.score_total + data.bonus) / data.target;

                        if (attack_count < enemy.attack - 30) {
                            continue;
                        }

                        attack.result.drop_garbage(enemy.attack - attack_count);

                        if (attack.result.get_height(2) >= 12) {
                            continue;
                        }

                        attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
                    }

                    for (auto& attack : c.attacks_detect) {
                        if (attack.frame > enemy.attack_frame) {
                            continue;
                        }

                        i32 attack_count = (attack.score_total + data.bonus) / data.target;

                        if (attack_count < enemy.attack - 30) {
                            continue;
                        }

                        attack.result.drop_garbage(enemy.attack - attack_count);

                        if (attack.result.get_height(2) >= 12) {
                            continue;
                        }

                        attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
                    }
                });

                std::vector<std::pair<Move::Placement, Search::Attack>> attacks;

                for (auto& c : search_result.candidates) {
                    for (auto& attack : c.attacks) {
                        if (attack.frame > enemy.attack_frame) {
                            continue;
                        }

                        if ((attack.score_total + data.bonus) / data.target < enemy.attack - 30) {
                            continue;
                        }

                        if (attack.result.get_height(2) >= 12) {
                            continue;
                        }

                        attacks.push_back({ c.placement, attack });
                    }

                    for (auto& attack : c.attacks_detect) {
                        if (attack.frame > enemy.attack_frame) {
                            continue;
                        }

                        if ((attack.score_total + data.bonus) / data.target < enemy.attack - 30) {
                            continue;
                        }

                        if (attack.result.get_height(2) >= 12) {
                            continue;
                        }

                        attacks.push_back({ c.placement, attack });
                    }
                }

                if (!attacks.empty()) {
                    auto best = *std::max_element(
                        attacks.begin(),
                        attacks.end(),
                        [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                            if (a.second.eval != b.second.eval) {
                                return a.second.eval < b.second.eval;
                            }

                            if (a.second.all_clear != b.second.all_clear) {
                                return a.second.all_clear < b.second.all_clear;
                            }

                            if (a.second.frame != b.second.frame) {
                                return a.second.frame > b.second.frame;
                            }

                            return a.second.count > b.second.count;
                        }
                    );

                    printf("l atk\n");
                    return Result {
                        .placement = best.first,
                        .eval = -1
                    };
                }
            }

            printf("run\n");

            if (enemy.attack_frame <= 4 && enemy.attack < 12 && field.get_height(2) < 8) {
                AI::get_candidate_eval(search_result, w.allclear);
                return AI::build(search_result, field, 78000, false);
            }

            if (field.get_count() >= 36 && enemy.attack_frame > 12) {
                AI::get_candidate_eval(search_result, w.build);
                return AI::build(search_result, field, 78000, false);
            }

            AI::get_candidate_eval(search_result, w.fast);
            return AI::build(search_result, field, 78000, false);
        }

        // Return
        auto return_condition = [&] (Search::Attack& attack) -> bool {
            if (attack.frame > enemy.attack_frame) {
                return false;
            }

            i32 attack_count = (attack.score_total + data.bonus) / data.target;
            if (attack_count < enemy.attack) {
                attack.result.drop_garbage(enemy.attack - attack_count);
            }

            if (attack.result.get_height(2) >= 11 || enemy.attack - attack_count > 6) {
                false;
            }

            return true;
        };

        AI::iterate_candidates(search_result, [&] (Search::Candidate& c) {
            for (auto& attack : c.attacks) {
                if (!return_condition(attack)) {
                    continue;
                }

                attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
            }

            for (auto& attack : c.attacks_detect) {
                if (!return_condition(attack)) {
                    continue;
                }

                attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
            }
        });

        std::vector<std::pair<Move::Placement, Search::Attack>> attacks;
        
        for (auto& c : search_result.candidates) {
            for (auto& attack : c.attacks) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                i32 attack_count = (attack.score_total + data.bonus) / data.target;
                
                if (attack.result.get_height(2) >= 11 || enemy.attack - attack_count > 6) {
                    false;
                }

                attacks.push_back({ c.placement, attack });
            }

            for (auto& attack : c.attacks_detect) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                i32 attack_count = (attack.score_total + data.bonus) / data.target;
                
                if (attack.result.get_height(2) >= 11 || enemy.attack - attack_count > 6) {
                    false;
                }

                attacks.push_back({ c.placement, attack });
            }
        }

        auto best = *std::max_element(
            attacks.begin(),
            attacks.end(),
            [&] (std::pair<Move::Placement, Search::Attack>& a, std::pair<Move::Placement, Search::Attack>& b) {
                bool a_main_chain = a.second.result.get_count() < field.get_count() / 2;
                bool b_main_chain = b.second.result.get_count() < field.get_count() / 2;

                if (a_main_chain && b_main_chain) {
                    if (a.second.score_total == b.second.score_total) {
                        a.second.frame > b.second.frame;
                    }

                    return a.second.score_total < b.second.score_total;
                }

                if (a_main_chain && !b_main_chain) {
                    return true;
                }

                if (!a_main_chain && b_main_chain) {
                    return false;
                }

                i32 a_over_enemy = (a.second.score + data.bonus) / data.target > enemy.attack;
                i32 b_over_enemy = (b.second.score + data.bonus) / data.target > enemy.attack;

                if (a_over_enemy != b_over_enemy) {
                    return a_over_enemy < b_over_enemy;
                }

                i32 a_over_enemy_big = (a.second.score + data.bonus) / data.target >= enemy.attack + 8;
                i32 b_over_enemy_big = (b.second.score + data.bonus) / data.target >= enemy.attack + 8;

                if (a_over_enemy_big != b_over_enemy_big) {
                    return a_over_enemy_big < b_over_enemy_big;
                }

                if (a.second.eval != b.second.eval) {
                    return a.second.eval < b.second.eval;
                }

                if (a.second.all_clear != b.second.all_clear) {
                    return a.second.all_clear < b.second.all_clear;
                }

                if (a.second.frame != b.second.frame) {
                    return a.second.frame > b.second.frame;
                }

                if (a.second.count != b.second.count) {
                    return a.second.count > b.second.count;
                }

                return a.second.score_total > b.second.score_total;
            }
        );

        printf("re\n");
        return Result {
            .placement = best.first,
            .eval = -1
        };
    }

    // Build all clear
    if (enemy.all_clear) {
        printf("ac\n");
        AI::get_candidate_eval(search_result, w.allclear);
        return AI::build(search_result, field, 70000);
    }

    // Kill
    if (AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
        printf("ko\n");
        i32 kill_needed = AI::get_unburied_count(enemy.field) + 72 - enemy.field.get_mask().get_mask_12().get_count();

        AI::iterate_candidates(search_result, [&] (Search::Candidate& c) {
            for (auto& attack : c.attacks) {
                if (attack.score + data.bonus >= kill_needed * data.target) {
                    continue;
                }

                attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
            }

            for (auto& attack : c.attacks_detect) {
                if (attack.score + data.bonus >= kill_needed * data.target) {
                    continue;
                }

                attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w.build);
            }
        });

        AI::get_candidate_eval(search_result, w.fast);

        return AI::build_attack(
            search_result,
            field,
            data,
            [&] (Search::Attack& attack) {
                return attack.score + data.bonus >= kill_needed * data.target;
            },
            [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                if (a.second.frame + a.second.count * 2 != b.second.frame + b.second.count * 2) {
                    return a.second.frame + a.second.count * 2 > b.second.frame + b.second.count * 2;
                }

                if (a.second.eval == b.second.eval) {
                    return a.second.frame > b.second.frame;
                }

                return a.second.eval < b.second.eval;
            }
        );
    }

    // Harass
    if (field.get_count() >= 42 && field.get_count() < 54) {
        bool trigger = false;

        for (auto& c : search_result.candidates) {
            for (auto& attack : c.attacks) {
                if (attack.score >= 78000) {
                    trigger = true;
                    break;
                }
            }

            if (trigger) {
                break;
            }
        }

        if (!trigger) {
            auto harass_condition = [&] (Search::Attack& attack) -> bool {
                if (attack.result.get_count() < 32 || attack.count > 2) {
                    return false;
                }

                auto attack_score = attack.score + data.bonus;

                if (attack_score + 4 * data.target < enemy_detect_harass.score + enemy.all_clear * 30 * data.target) {
                    return false;
                }

                if (attack.count == 1 && attack_score >= 420) {
                    return true;
                }

                if (attack.count == 2 && attack_score >= 840) {
                    return true;
                }

                if (attack.count == 3 && attack_score >= 1680) {
                    return true;
                }

                return false;
            };

            AI::iterate_candidates(search_result, [&] (Search::Candidate& c) {
                for (auto& attack : c.attacks) {
                    if (!harass_condition(attack)) {
                        continue;
                    }

                    attack.eval = Eval::evaluate(attack.result, 0, 0, 3, w.build);
                }

                // for (auto& attack : c.attacks_detect) {
                //     if (!harass_condition(attack)) {
                //         continue;
                //     }

                //     attack.eval = Eval::evaluate(attack.result, 0, 0, 4, w.build);
                // }
            });

            std::vector<std::pair<Move::Placement, Search::Attack>> attacks;

            for (auto& c : search_result.candidates) {
                for (auto& attack : c.attacks) {
                    if (!harass_condition(attack)) {
                        continue;
                    }

                    attacks.push_back({ c.placement, attack });
                }

                // for (auto& attack : c.attacks_detect) {
                //     if (!harass_condition(attack)) {
                //         continue;
                //     }

                //     attacks.push_back({ c.placement, attack });
                // }
            }

            if (!attacks.empty()) {
                auto best = *std::max_element(
                    attacks.begin(),
                    attacks.end(),
                    [&] (std::pair<Move::Placement, Search::Attack>& a, std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.eval == b.second.eval) {
                            return a.second.frame > b.second.frame;
                        }

                        return a.second.eval < b.second.eval;
                    }
                );

                printf("har\n");
                return Result {
                    .placement = best.first,
                    .eval = -1
                };
            }
        }
    }

    if (AI::get_small_field(enemy.field, field)) {
        AI::get_candidate_eval(search_result, w.fast);
        return AI::build(search_result, field, 70000);
    }

    // Build
    AI::get_candidate_eval(search_result, w.build);
    return AI::build(search_result, field, 78000);
};

Result build(Search::Result& search, Field& field, i32 trigger, bool all_clear)
{
    if (all_clear && field.get_count() <= 20) {
        std::pair<Move::Placement, Search::Attack> best = { Move::Placement(), Search::Attack()};

        for (auto& c : search.candidates) {
            for (auto& attack : c.attacks) {
                if (!attack.all_clear || attack.frame > 4 || attack.count > 4) {
                    continue;
                }

                best = std::max(
                    best,
                    { c.placement, attack },
                    [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.score == b.second.score) {
                            return a.second.frame_total > b.second.frame_total;
                        }
                        return a.second.score < b.second.score;
                    }
                );
            }
        }

        if (best.second.count > 0) {
            return Result {
                .placement = best.first,
                .eval = 0,
            };
        }
    }

    i32 chain_score_max = 0;
    i32 attack_count = 0;

    for (auto& candidate : search.candidates) {
        for (auto& attack : candidate.attacks) {
            chain_score_max = std::max(chain_score_max, attack.score);
        }
        attack_count += candidate.attacks.size();
    }

    bool build = chain_score_max < trigger;

    if (build) {
        auto best = *std::max_element(
            search.candidates.begin(),
            search.candidates.end(),
            [&] (const Search::Candidate& a, const Search::Candidate& b) {
                if (a.eval != b.eval) {
                    return a.eval < b.eval;
                }

                if (a.eval_fast != b.eval_fast) {
                    return a.eval_fast < b.eval_fast;
                }

                return a.attacks.size() < b.attacks.size();
            }
        );
        
        if (best.eval > INT32_MIN) {
            return Result {
                .placement = best.placement,
                .eval = best.eval,
            };
        }
    }

    if (attack_count > 0) {
        std::pair<i32, Result> best = { 0, Result() };

        for (auto& c : search.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            auto attack = *std::max_element(
                c.attacks.begin(),
                c.attacks.end(),
                [&] (const Search::Attack& a, const Search::Attack& b) {
                    if (a.score == b.score) {
                        return a.frame > b.frame;
                    }
                    
                    return a.score < b.score;
                }
            );

            if (best.first < attack.score) {
                best.first = attack.score;
                best.second.placement = c.placement;
                best.second.eval = 0;
            }
        }

        return best.second;
    }

    return AI::RESULT_DEFAULT;
};

Result build_attack(
    Search::Result& search,
    Field& field,
    Data data,
    std::function<bool(Search::Attack&)> condition,
    std::function<bool(const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b)> cmp,
    bool all_clear
)
{
    if (all_clear && field.get_count() <= 20) {
        std::pair<Move::Placement, Search::Attack> best = { Move::Placement(), Search::Attack()};

        for (auto& c : search.candidates) {
            for (auto& attack : c.attacks) {
                if (!attack.all_clear || attack.frame > 4 || attack.count > 4) {
                    continue;
                }

                best = std::max(
                    best,
                    { c.placement, attack },
                    [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.score == b.second.score) {
                            return a.second.frame_total > b.second.frame_total;
                        }
                        return a.second.score < b.second.score;
                    }
                );
            }
        }

        if (best.second.count > 0) {
            return Result {
                .placement = best.first,
                .eval = 0,
            };
        }
    }

    std::vector<std::pair<Move::Placement, Search::Attack>> attacks;

    for (auto& c : search.candidates) {
        for (auto& attack : c.attacks) {
            if (condition(attack)) {
                attacks.push_back({ c.placement, attack });
            }
        }

        for (auto& attack : c.attacks_detect) {
            if (condition(attack)) {
                attacks.push_back({ c.placement, attack });
            }
        }
    }

    if (!attacks.empty()) {
        auto best = *std::max_element(
            attacks.begin(),
            attacks.end(),
            cmp
        );

        return Result {
            .placement = best.first,
            .eval = 0,
        };
    }

    auto best = *std::max_element(
        search.candidates.begin(),
        search.candidates.end(),
        [&] (const Search::Candidate& a, const Search::Candidate& b) {
            if (a.eval != b.eval) {
                return a.eval < b.eval;
            }

            if (a.eval_fast != b.eval_fast) {
                return a.eval_fast < b.eval_fast;
            }

            return a.attacks.size() < b.attacks.size();
        }
    );
    
    if (best.eval > INT32_MIN) {
        return Result {
            .placement = best.placement,
            .eval = best.eval,
        };
    }

    return AI::RESULT_DEFAULT;
};

void get_candidate_eval(Search::Result& search, Eval::Weight w)
{
    AI::iterate_candidates(search, [&] (Search::Candidate& c) {
        c.eval_fast = Eval::evaluate(c.plan_fast.field, c.plan_fast.tear, c.plan_fast.waste, 8, w);
        c.eval = INT32_MIN;
        
        for (auto& p : c.plans) {
            c.eval = std::max(c.eval, Eval::evaluate(p.field, p.tear, p.waste, 8, w));
        }
    });
};

void get_attacks_eval(Search::Result& search, Eval::Weight w)
{
    AI::iterate_candidates(search, [&] (Search::Candidate& c) {
        for (auto& attack : c.attacks) {
            attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w);
        }

        for (auto& attack : c.attacks_detect) {
            attack.eval = Eval::evaluate(attack.result, 0, 0, 2, w);
        }
    });
};

void iterate_candidates(Search::Result& search, std::function<void(Search::Candidate&)> func)
{
    std::mutex mtx;
    std::vector<std::thread> threads;

    i32 id = 0;

    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] () {
            while (true)
            {
                i32 t_id = 0;

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    if (id >= search.candidates.size()) {
                        break;
                    }
                    t_id = id;
                    id += 1;
                }

                func(search.candidates[t_id]);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
};

i32 get_unburied_count(Field& field)
{
    auto mask = field.get_mask();
    auto mask_empty = ~mask;
    auto mask_color = mask & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)]);
    auto mask_above = (mask_empty | mask_color).get_mask_group(2, 11);
    auto mask_unburied = mask_above & mask_color;

    return mask_unburied.get_count();
};

bool get_garbage_obstruct(Field& field, Chain::Score& detect_highest, Chain::Score& detect_harass)
{
    i32 unburied_count = AI::get_unburied_count(field);
    i32 garbage_count = field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();
    i32 empty_count = (~field.get_mask()).get_mask_group(2, 11).get_count();

    if (garbage_count < 1) {
        return false;
    }

    return
        (garbage_count >= 30) ||
        (garbage_count >= (field.get_count() / 2)) ||
        (garbage_count >= 12 && unburied_count <= garbage_count) ||
        (garbage_count >= 12 && detect_highest.score <= 630);
};

bool get_small_field(Field& field, Field& other)
{
    i32 field_count = (field.get_mask() & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();
    i32 other_count = (other.get_mask() & (~other.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();

    return (other_count > field_count * 2) && other_count >= 22;
};

bool get_bad_field(Field& field)
{
    u8 heights[6];
    field.get_heights(heights);

    auto height_diff = *std::max_element(heights, heights + 6) - *std::max_element(heights, heights + 6);

    return heights[2] > 10 || height_diff <= 1;
};

void get_gaze_field(Field& field, std::vector<Cell::Pair> queue, Chain::Score& detect_highest, Chain::Score& detect_harass)
{
    detect_highest = { 0, 0 };
    detect_harass = { 0, 0 };

    Qsearch::search(field, 8, 3, [&] (Qsearch::Result detect) {
        if (detect.score > detect_highest.score) {
            detect_highest = {
                detect.chain,
                detect.score
            };
        }
    });

    Qsearch::search(field, 1, 2, [&] (Qsearch::Result detect) {
        if (detect.chain < 3 && detect.score > detect_harass.score) {
            detect_harass = Chain::Score {
                .count = detect.chain,
                .score = detect.score
            };
        }
    });
};

};