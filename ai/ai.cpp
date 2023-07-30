#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 trigger_score)
{
    auto search_result = Search::search(field, queue);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1
        };
    }

    AI::get_candidate_eval(search_result, w);
    return AI::build(search_result, field, trigger_score);
};

Result build(Search::Result& search_result, Field& field, i32 trigger_score)
{
    // All clear
#ifdef TUNER
#else
    auto all_clear_attacks = AI::get_attacks_with_condition(search_result, [] (Search::Attack& attack) {
        return attack.frame <= 4 && attack.count <= 4 && attack.all_clear;
    });

    if (!all_clear_attacks.empty()) {
        std::sort(
            all_clear_attacks.begin(),
            all_clear_attacks.end(),
            [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                if (a.second.score == b.second.score) {
                    return a.second.frame < b.second.frame;
                }
                return a.second.score > b.second.score;
            }
        );
        return Result {
            .placement = all_clear_attacks[0].first,
            .eval = 0,
        };
    }
#endif

    // Check trigger chain condition
    i32 chain_score_max = 0;
    i32 attack_count = 0;
    for (auto& candidate : search_result.candidates) {
        for (auto& attack : candidate.attacks) {
            chain_score_max = std::max(chain_score_max, attack.score);
        }
        attack_count += candidate.attacks.size();
    }
    bool trigger = chain_score_max >= trigger_score && field.get_count() >= 56;

    // Build chain
    if (!trigger) {
        auto best = *std::max_element(
            search_result.candidates.begin(),
            search_result.candidates.end(),
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

    // Trigger chain
    if (attack_count > 0) {
        std::pair<i32, Result> best = { 0, Result() };

        for (auto& c : search_result.candidates) {
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

    // Error
    return Result {
        .placement = { .x = 2, .r = Direction::Type::UP },
        .eval = -2
    };
};

Result build_attack(Search::Result& search_result, Field& field, Data data, std::function<bool(Search::Attack&)> condition)
{
    // Trigger all clear
    auto all_clear_attacks = AI::get_attacks_with_condition(search_result, [] (Search::Attack& attack) {
        return attack.frame <= 4 && attack.count <= 4 && attack.all_clear;
    });

    if (!all_clear_attacks.empty()) {
        std::sort(
            all_clear_attacks.begin(),
            all_clear_attacks.end(),
            [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                if (a.second.score == b.second.score) {
                    return a.second.frame < b.second.frame;
                }
                return a.second.score > b.second.score;
            }
        );
        
        return Result {
            .placement = all_clear_attacks[0].first,
            .eval = 0,
        };
    }

    // Trigger best attack
    std::vector<std::pair<Move::Placement, Search::Attack>> attacks;

    for (auto& c : search_result.candidates) {
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
            [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                if (a.second.eval == b.second.eval) {
                    return a.second.frame > b.second.frame;
                }

                return a.second.eval < b.second.eval;
            }
        );

        return Result {
            .placement = best.first,
            .eval = 0,
        };
    }

    // Else build chain
    auto best = *std::max_element(
        search_result.candidates.begin(),
        search_result.candidates.end(),
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

    // Error
    return Result {
        .placement = { .x = 2, .r = Direction::Type::UP },
        .eval = -2
    };
};

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w)
{
    auto search_result = Search::search(field, queue);

    if (search_result.candidates.empty()) {
        printf("die\n");
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1
        };
    }

    // Active gaze
    Chain::Score enemy_detect_highest;
    Chain::Score enemy_detect_harass;
    AI::get_gaze_field(enemy.field, enemy_detect_highest, enemy_detect_harass);

    // If enemy is attacking
    if (enemy.attack > 0) {
        // Enemy is having all clear, then try to find all clear
        if (enemy.all_clear) {
            auto attacks_all_clear = AI::get_attacks_with_condition(search_result, [&] (Search::Attack& attack) {
                return attack.all_clear && attack.frame <= enemy.attack_frame;
            });

            if (!attacks_all_clear.empty()) {
                auto best = *std::max_element(
                    attacks_all_clear.begin(),
                    attacks_all_clear.end(),
                    [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.score_total == b.second.score_total) {
                            return a.second.frame > b.second.frame;
                        }
                        return a.second.score_total < b.second.score_total;
                    }
                );

                printf("ac re\n");
                return Result {
                    .placement = best.first,
                    .eval = 0
                };
            }
        }

        // Try to receive small garbage
        if (!data.all_clear &&
            AI::get_can_receive_garbage(field, enemy) &&
            field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() < 9 &&
            !AI::get_small_field(enemy.field, field) &&
            !AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
            printf("accept\n");
            AI::get_candidate_eval(search_result, Eval::FAST_WEIGHT);
            return AI::build(search_result, field, 71000);
        }

        // Evaluate fields remained after triggered chains
        AI::get_attacks_eval(search_result, w);

        // Find all attacks within the remaining time
        std::vector<std::pair<Move::Placement, Search::Attack>> attacks_return;
        std::vector<std::pair<Move::Placement, Search::Attack>> attacks_in_frame;

        i32 attacks_in_frame_best = 0;

        for (auto& c : search_result.candidates) {
            for (auto& attack : c.attacks) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                attacks_in_frame.push_back({ c.placement, attack });

                if ((attack.score_total + data.bonus) / data.target + data.all_clear * 30 >= enemy.attack) {
                    attacks_return.push_back({ c.placement, attack});
                }

                attacks_in_frame_best = std::max(attacks_in_frame_best, attack.score);
            }

            for (auto& attack : c.attacks_detect) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                attacks_in_frame.push_back({ c.placement, attack });

                if ((attack.score_total + data.bonus) / data.target + data.all_clear * 30 >= enemy.attack) {
                    attacks_return.push_back({ c.placement, attack});
                }

                attacks_in_frame_best = std::max(attacks_in_frame_best, attack.score);
            }
        }

        // If there aren't any returnable attacks
        if (attacks_return.empty()) {
            // And if there are still some attacks (but can't offset) and the remaining time is small
            if ((enemy.attack_frame <= 4) && (!attacks_in_frame.empty())) {
                // Then trigger the biggest possible chain
                auto best = *std::max_element(
                    attacks_in_frame.begin(),
                    attacks_in_frame.end(),
                    [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.score_total == b.second.score_total) {
                            return a.second.frame > b.second.frame;
                        }
                        return a.second.score_total < b.second.score_total;
                    }
                );

                if ((best.second.score_total + data.bonus) / data.target + data.all_clear * 30 >= enemy.attack - 30) {
                    printf("last ditch\n");
                    return Result {
                        .placement = best.first,
                        .eval = -1
                    };
                }
            }
            
            // If there aren't any possible offset attacks but the remaining time is large, then try to build chain fast
            printf("catch\n");
            auto weight_build = Eval::FAST_WEIGHT;
            if (field.get_count() >= 48 && enemy.attack >= 60) {
                weight_build = w;
            }
            AI::get_candidate_eval(search_result, weight_build);
            return AI::build(search_result, field, 71000);
        }
        else {
            // Return the enemy's attack with the biggest chain if:
            //  - The enemy's attack is too big
            //  - Our main chain is big enough
            //  - Our field is big enough
            //  - The enemy just triggered their biggest chain
            //  - The enemy is in danger
            if (enemy.attack >= 60 ||
                attacks_in_frame_best >= 71000 ||
                field.get_count() >= 60 ||
                AI::get_small_field(enemy.field, field) ||
                AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
                auto best = *std::max_element(
                    attacks_return.begin(),
                    attacks_return.end(),
                    [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                        if (a.second.score_total == b.second.score_total) {
                            return a.second.frame > b.second.frame;
                        }
                        return a.second.score_total < b.second.score_total;
                    }
                );

                printf("re big\n");
                return Result {
                    .placement = best.first,
                    .eval = -1
                };
            }

            // Else return some harassment
            auto best = *std::max_element(
                attacks_return.begin(),
                attacks_return.end(),
                [&] (const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b) {
                    bool a_main_chain = a.second.score_total > 5000;
                    bool b_main_chain = b.second.score_total > 5000;

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

            printf("re smol\n");
            return Result {
                .placement = best.first,
                .eval = -1
            };
        }
    }

    // If we are in danger or the enemy is having all clear, them build fast and safely
    if (enemy.all_clear) {
        printf("oppo ac\n");
        AI::get_attacks_eval(search_result, w);
        AI::get_candidate_eval(search_result, Eval::FAST_WEIGHT);
        return AI::build(search_result, field, 71000);
    }

    // If the enemy is being obstructed by garbage
    if (AI::get_garbage_obstruct(enemy.field, enemy_detect_highest, enemy_detect_harass)) {
        printf("gb kill\n");
        // Build fast and trigger chain fast
        AI::get_attacks_eval(search_result, w);
        AI::get_candidate_eval(search_result, Eval::FAST_WEIGHT);
        return AI::build(search_result, field, 2100);
    }

    // Else, if our enemy's field is smaller than our's field a lot, then harass them
    // if (AI::get_small_field(enemy.field, field)) {
    //     printf("build harass small field\n");
    //     // Trigger harassment
    //     AI::get_attacks_eval(search_result, w);
    //     AI::get_candidate_eval(search_result, Eval::FAST_WEIGHT);
    //     return AI::build_attack(search_result, field, data, [&] (Search::Attack& attack) {
    //         return attack.score + data.bonus + data.all_clear * 30 * data.target >= 1000;
    //     });
    // }

    // Try harass
    if (field.get_count() >= 42 && field.get_count() < 60) {
        printf("harass\n");
        AI::get_attacks_eval(search_result, w);
        AI::get_candidate_eval(search_result, w);
        return AI::build_attack(search_result, field, data, [&] (Search::Attack& attack) {
            if (attack.result.get_count() < 36) {
                return false;
            }

            auto attack_score = attack.score + data.bonus + data.all_clear * 30 * data.target;

            if (attack_score < enemy_detect_harass.score - 140) {
                return false;
            }

            if (attack.count == 1 && attack_score >= 420) {
                return true;
            }

            if (attack.count == 2 && attack_score >= 630) {
                return true;
            }

            if (attack.count == 3 && attack_score >= 1200) {
                return true;
            }

            if (attack.count == 4 || attack.count == 5) {
                return true;
            }

            return false;
        });
    }

    // Else build chain
    AI::get_candidate_eval(search_result, w);
    return AI::build(search_result, field, 71000);
};

void get_candidate_eval(Search::Result& search_result, Eval::Weight w)
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
                    if (id >= search_result.candidates.size()) {
                        break;
                    }
                    t_id = id;
                    id += 1;
                }

                auto& c = search_result.candidates[t_id];

                c.eval_fast = Eval::evaluate(c.plan_fast.field, c.plan_fast.tear, c.plan_fast.waste, w);

                for (auto& p : c.plans) {
                    c.eval = std::max(
                        c.eval,
                        Eval::evaluate(p.field, p.tear, p.waste, w)
                    );
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
};

void get_attacks_eval(Search::Result& search_result, Eval::Weight w)
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
                    if (id >= search_result.candidates.size()) {
                        break;
                    }
                    t_id = id;
                    id += 1;
                }

                auto& c = search_result.candidates[t_id];

                for (auto& attack : c.attacks) {
                    attack.eval = Eval::evaluate(attack.result, 0, 0, w);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
};

std::vector<std::pair<Move::Placement, Search::Attack>> get_attacks_with_condition(Search::Result& search_result, std::function<bool(Search::Attack&)> condition)
{
    std::vector<std::pair<Move::Placement, Search::Attack>> result;

    std::mutex mtx;
    std::vector<std::thread> threads;

    i32 id = 0;

    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] () {
            std::vector<std::pair<Move::Placement, Search::Attack>> t_result;

            while (true)
            {
                i32 t_id = 0;

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    if (id >= search_result.candidates.size()) {
                        break;
                    }
                    t_id = id;
                    id += 1;
                }

                auto& c = search_result.candidates[t_id];

                for (auto& attack : c.attacks) {
                    if (condition(attack)) {
                        t_result.push_back({ c.placement, attack });
                    }
                }

                for (auto& attack : c.attacks_detect) {
                    if (condition(attack)) {
                        t_result.push_back({ c.placement, attack });
                    }
                }
            }

            std::lock_guard<std::mutex> lk(mtx);
            result.insert(result.end(), t_result.begin(), t_result.end());
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
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
        (garbage_count >= 18) ||
        (garbage_count >= (field.get_count() / 2)) ||
        (unburied_count < 16 && garbage_count >= 12) ||
        (unburied_count <= garbage_count) ||
        (garbage_count >= 12 && detect_highest.score <= 630);
};

bool get_small_field(Field& field, Field& other)
{
    i32 field_count = (field.get_mask() & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();
    i32 other_count = (other.get_mask() & (~other.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();

    return
        (other_count > field_count * 2) ||
        (other_count >= 30 && field_count <= 12);
};

bool get_bad_field(Field& field)
{
    u8 heights[6];
    field.get_heights(heights);

    auto height_diff = *std::max_element(heights, heights + 6) - *std::max_element(heights, heights + 6);

    return heights[2] > 10 || height_diff <= 1;
};

bool get_can_receive_garbage(Field& field, Enemy& enemy)
{
    // if (enemy.attack > 12) {
    //     return false;
    // }

    // if (field.get_height(2) >= 10) {
    //     return false;
    // }

    // auto count = field.get_count();

    // if (count > 56) {
    //     return false;
    // }

    // if (field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() >= 9) {
    //     return false;
    // }

    // if (enemy.attack <= 3) {
    //     return true;
    // }

    // u8 heights[6];
    // field.get_heights(heights);

    // auto height_diff = *std::max_element(heights, heights + 6) - *std::max_element(heights, heights + 6);
    // if (height_diff <= 1) {
    //     return false;
    // }

    // if (count < 30) {
    //     return enemy.attack <= 9;
    // }
    // else if (count < 48) {
    //     return enemy.attack <= 6;
    // }

    // return false;

    if (field.get_height(2) >= 10) {
        return false;
    }

    auto count = field.get_count();

    if (count <= 30) {
        return enemy.attack <= 9;
    }
    else if (count <= 48) {
        return enemy.attack <= 6;
    }
    else if (count <= 54) {
        return enemy.attack <= 4;
    }

    return false;
};

void get_gaze_field(Field& field, Chain::Score& detect_highest, Chain::Score& detect_harass)
{
    detect_highest = { 0, 0 };
    detect_harass = { 0, 0 };

    Detect::detect(field, [&] (Detect::Result detect) {
        if (detect.score.chain.score > detect_highest.score) {
            detect_highest = detect.score.chain;
        }

        if (detect.score.chain.count < 6 && detect.score.chain.score > detect_harass.score) {
            detect_harass = detect.score.chain;
        }
    }, 2, 2);
};

};