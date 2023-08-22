#pragma once

#include "fieldbit.h"

class Field
{
public:
    FieldBit data[Cell::COUNT];
public:
    Field();
public:
    bool operator == (const Field& other);
    bool operator != (const Field& other);
public:
    void set_cell(i8 x, i8 y, Cell::Type cell);
    Cell::Type get_cell(i8 x, i8 y);
public:
    u32 get_count();
    u8 get_height(i8 x);
    u8 get_height_max();
    void get_heights(u8 heights[6]);
    FieldBit get_mask();
    Field get_mask_pop();
    u8 get_drop_pair_frame(i8 x, Direction::Type direction);
public:
    bool is_occupied(i8 x, i8 y);
    bool is_occupied(i8 x, i8 y, u8 heights[6]);
    bool is_colliding_pair(i8 x, i8 y, Direction::Type direction);
    bool is_colliding_pair(i8 x, i8 y, Direction::Type direction, u8 heights[6]);
    bool is_empty();
public:
    void drop_puyo(i8 x, Cell::Type cell);
    void drop_pair(i8 x, Direction::Type direction, Cell::Pair pair);
    void drop_garbage(i32 count);
public:
    avec<Field, 19> pop();
public:
    void from(const char c[13][7]);
    void print();
};

namespace Chain
{

struct Score
{
    i32 count = 0;
    i32 score = 0;
};

constexpr u32 COLOR_BONUS[] = { 0, 0, 3, 6, 12, 24 };
constexpr u32 GROUP_BONUS[] = { 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10 };
constexpr u32 POWER[] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };

static Score get_score(avec<Field, 19>& mask)
{
    Score result = {
        .count = mask.get_size(),
        .score = 0
    };

    for (i32 index = 0; index < mask.get_size(); ++index) {
        u32 pop_count = mask[index].get_count();

        u32 chain_power = Chain::POWER[index];

        u32 color = 0;
        for (u8 cell = 0; cell < Cell::COUNT - 1; ++cell) {
            color += mask[index].data[cell].get_count() > 0;
        }
        u32 bonus_color = Chain::COLOR_BONUS[color];

        u32 group_bonus = 0;
        for (u8 cell = 0; cell < Cell::COUNT - 1; ++cell) {
            while (mask[index].data[cell].get_count() > 0)
            {
                FieldBit group = mask[index].data[cell].get_mask_group_lsb();
                mask[index].data[cell] = mask[index].data[cell] & (~group);
                group_bonus += Chain::GROUP_BONUS[std::min(11U, group.get_count())];
            }
        }

        result.score += pop_count * 10 * std::clamp(chain_power + bonus_color + group_bonus, 1U, 999U);
    }

    return result;
};

};

static i64 bench_pop(i32 iter)
{
    Field f;
    const char c[13][7] = {
        "B.YRGY",
        "BBBYRB",
        "GBYRGG",
        "BGYRGB",
        "GRGYRB",
        "RGYRYB",
        "GRGYRY",
        "GRGYRY",
        "GBBGYG",
        "BYRBGG",
        "GBYRBY",
        "GBYRBY",
        "GBYRBY",
    };
    f.from(c);
    f.print();

    i64 time = 0;
    avec<Field, 19> mask = avec<Field, 19>();

    for (i32 i = 0; i < iter; ++i) {
        auto f_copy = f;
        auto time_start = std::chrono::high_resolution_clock::now();
        mask = f_copy.pop();
        auto time_end = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    }

    auto chain = Chain::get_score(mask);

    std::cout << "count: " << int(chain.count) << std::endl;
    std::cout << "score: " << int(chain.score) << std::endl;

    return time / iter;
};