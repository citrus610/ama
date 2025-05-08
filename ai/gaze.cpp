#include "gaze.h"

namespace gaze
{

// Looks at the opponent's field and see their options
Data gaze(Field& field, dfs::attack::Result& asearch, i32 delay)
{
    Data result = Data();

    i32 field_count = field.get_count();

    // Max acceptable chain redundancy
    const i32 REDUNDANCY_MAX = 5;

    // Options' types condition
    // Harass attacks
    const auto harass_condition = [&] (dfs::attack::Data& attack) -> bool {
        return attack.count < 4;
    };

    // Early quick attacks
    const auto early_condition = [&] (dfs::attack::Data& attack, i32 remain) -> bool {
        return attack.count <= 2 && remain <= 6;
    };

    // Defense level 1
    const auto defence_1_condition = [&] (dfs::attack::Data& attack, i32 remain) -> bool {
        return attack.count <= 3 && attack.frame <= 1 && remain >= std::max(24, field_count / 2);
    };

    // Defense level 2
    const auto defence_2_condition = [&] (dfs::attack::Data& attack, i32 remain) -> bool {
        return attack.count <= 6 && attack.frame <= 4 && remain >= std::max(24, field_count / 2);
    };

    // Function for sorting the options
    const auto cmp_level_1 = [&] (const dfs::attack::Data& a, const dfs::attack::Data& b) {
        if (a.frame + 2 * a.count != b.frame + 2 * b.count) {
            return a.frame + 2 * a.count > b.frame + 2 * b.count;
        }

        if (a.score != b.score) {
            return a.score < b.score;
        }

        if (a.link != b.link) {
            return a.link < b.link;
        }

        return a.frame_real < b.frame_real;
    };

    const auto cmp_level_2 = [&] (const dfs::attack::Data& a, const dfs::attack::Data& b) {
        if (a.score != b.score) {
            return a.score < b.score;
        }

        if (a.link != b.link) {
            return a.link < b.link;
        }

        return a.frame_real < b.frame_real;
    };

    // Finds options
    // Evaluation function
    const auto evaluate_attack = [&] (dfs::attack::Data& attack) {
        // Adds delay
        attack.frame += delay;
        attack.frame_real += delay;

        // Gets attack's redundancy
        attack.redundancy = gaze::get_redundancy(attack.parent, attack.result);

        // Gets main chain
        if (attack.score > result.main.score) {
            result.main = attack;
        }

        // Checks redundancy
        if (attack.redundancy > REDUNDANCY_MAX) {
            return;
        }

        // Attack's remain count
        i32 remain = attack.result.get_count();

        // Gets harass
        if (harass_condition(attack)) {
            result.harass = std::max(
                result.harass,
                attack,
                cmp_level_1
            );
        }

        // Gets defense level 1
        if (defence_1_condition(attack, remain)) {
            result.defence_1 = std::max(
                result.defence_1,
                attack,
                cmp_level_1
            );
        }

        // Gets defense level 2
        if (defence_2_condition(attack, remain)) {
            result.defence_2 = std::max(
                result.defence_2,
                attack,
                cmp_level_2
            );
        }
    };

    // Iterates all attacks
    for (auto& c : asearch.candidates) {
        for (auto& attack : c.attacks) {
            evaluate_attack(attack);
        }

        for (auto& attack : c.attacks_detect) {
            evaluate_attack(attack);
        }
    }

    // Finds best main chain future
    dfs::quiet::search(field, 8, 3, [&] (dfs::quiet::Result q) {
        if (q.chain.score > result.main_q.score) {
            result.main_q.score = q.chain.score;
        }
    });

    return result;
};

// Gets the number of connected puyo with the sky
i32 get_unburied_count(Field& field)
{
    auto mask = field.get_mask();
    auto mask_empty = ~mask;
    auto mask_color = mask & (~field.data[static_cast<i32>(cell::Type::GARBAGE)]);
    auto mask_above = (mask_empty | mask_color).get_mask_group(2, 11);
    auto mask_unburied = mask_above & mask_color;

    return mask_unburied.get_count();
};

// Gets the acceptable amount of garbage puyos that we can receive
i32 get_accept_limit(Field& field)
{
    i32 result = 0;

    i32 field_count = field.get_count();
    i32 field_count_left = field.get_height(0) + field.get_height(1) + field.get_height(2);

    if (field.get_height(2) > 9) {
        return 0;
    }

    if (field_count >= 48) {
        result = 0;
    }
    else if (field_count >= 36) {
        result = 6;
    }
    else if (field_count >= 30) {
        if (field.get_height(2) <= 4) {
            result = 9;
        }
        else {
            result = 6;
        }
    }
    else {
        if (field.get_height(2) <= 1) {
            result = 30;
        }
        else {
            result = 18;
        }
    }

    i32 garbage_count = field.data[static_cast<i32>(cell::Type::GARBAGE)].get_count();

    return std::max(0, result - garbage_count);
};

// Gets an attack's redundancy
// An attack's redundancy is defined as the number of puyos cell that are dropped in the chain
//
// Ex:
// ......    ......    ......
// B.....    B.....    ......
// R..... -> ...... -> ......
// RGYY..    ..YY..    ......
// RRGG..    ......    ..YY..
// GGRR..    ..RR..    B.RR..
// 
// In the above example, we triggered a 2-chain, popping the red group then the green group
// After the chain finished, 1 blue puyo and 2 yellow puyos dropped, while 2 red puyos stayed the same
// We conclude that this attack/chain has a redundancy of 3, because there was 3 puyos dropped (1 blue puyo and 3 yellows)
i32 get_redundancy(Field& pre, Field& now)
{
    i32 result = 0;

    u8 heights_pre[6];
    u8 heights_now[6];

    pre.get_heights(heights_pre);
    now.get_heights(heights_now);

    i32 heights[6];
    
    for (i32 i = 0; i < 6; ++i) {
        heights[i] = std::min(heights_pre[i], heights_now[i]);
    }

    for (i32 x = 0; x < 6; ++x) {
        for (i32 y = 0; y < heights[x]; ++y) {
            auto pre_cell = pre.get_cell(x, y);

            if (!now.data[static_cast<i32>(pre_cell)].get_bit(x, y)) {
                result += heights[x] - y;
                break;
            }
        }
    }

    return result;
};

// Checks if a field is being obstructed by garbages
bool is_garbage_obstruct(Field& field, chain::Score detect_highest)
{
    i32 unburied_count = gaze::get_unburied_count(field);
    i32 garbage_count = field.data[static_cast<i32>(cell::Type::GARBAGE)].get_count();
    i32 empty_count = (~field.get_mask()).get_mask_group(2, 11).get_count();

    if (garbage_count < 1) {
        return false;
    }

    return
        (garbage_count >= 24) ||
        (garbage_count >= (field.get_count() / 2)) ||
        (garbage_count >= 12 && unburied_count <= garbage_count) ||
        (garbage_count >= 12 && detect_highest.score <= 630);
};

bool is_small_field(Field& field, Field& other)
{
    i32 field_count = (field.get_mask() & (~field.data[static_cast<i32>(cell::Type::GARBAGE)])).get_count();
    i32 other_count = (other.get_mask() & (~other.data[static_cast<i32>(cell::Type::GARBAGE)])).get_count();

    return (other_count >= field_count * 2 || field_count <= 24) && other_count >= 22;
};

i32 get_resource_balance(Field& field, Field& other)
{
    i32 unburied_count_field = gaze::get_unburied_count(field);
    i32 unburied_count_other = gaze::get_unburied_count(other);

    i32 field_count = field.get_count();
    i32 other_count = other.get_count();

    if (std::abs(unburied_count_field - unburied_count_other) >= 12) {
        return unburied_count_field - unburied_count_other;
    }

    return field_count - other_count;
};

};