#pragma once

#include "field.h"

namespace chain
{

struct Score
{
    i32 count = 0;
    i32 score = 0;
};

constexpr u32 COLOR_BONUS[] = { 0, 0, 3, 6, 12, 24 };
constexpr u32 GROUP_BONUS[] = { 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10 };
constexpr u32 POWER[] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };

// Returns the score of a chain
inline Score get_score(avec<Field, 19>& mask)
{
    Score result = {
        .count = mask.get_size(),
        .score = 0
    };

    for (i32 index = 0; index < mask.get_size(); ++index) {
        // Puyo popped count
        u32 pop_count = mask[index].get_count();

        // Chain's power
        u32 chain_power = chain::POWER[index];

        // Chain's color bonus
        u32 color = 0;

        for (u8 cell = 0; cell < cell::COUNT - 1; ++cell) {
            color += !mask[index].data[cell].is_empty();
        }

        u32 bonus_color = chain::COLOR_BONUS[color];

        // Chain's group bonus
        u32 group_bonus = 0;

        for (u8 cell = 0; cell < cell::COUNT - 1; ++cell) {
            while (!mask[index].data[cell].is_empty())
            {
                auto group = mask[index].data[cell].get_mask_group_lsb();
                mask[index].data[cell] = mask[index].data[cell] & (~group);
                group_bonus += chain::GROUP_BONUS[std::min(11U, group.get_count())];
            }
        }

        // Accumulate chain score
        result.score += pop_count * 10 * std::clamp(chain_power + bonus_color + group_bonus, 1U, 999U);
    }

    return result;
};

};