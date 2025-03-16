#pragma once

#include "fieldbit.h"
#include "avec.h"

class Field
{
public:
    FieldBit data[cell::COUNT];
    u8 row14;
public:
    Field();
public:
    bool operator == (const Field& other);
    bool operator != (const Field& other);
public:
    void set_cell(i8 x, i8 y, cell::Type cell);
    cell::Type get_cell(i8 x, i8 y);
public:
    u32 get_count();
    u8 get_height(i8 x);
    u8 get_height_max();
    void get_heights(u8 heights[6]);
    FieldBit get_mask();
    Field get_mask_pop();
    u8 get_drop_pair_frame(i8 x, direction::Type direction);
public:
    bool is_occupied(i8 x, i8 y);
    bool is_occupied(i8 x, i8 y, u8 heights[6]);
    bool is_colliding_pair(i8 x, i8 y, direction::Type direction);
    bool is_colliding_pair(i8 x, i8 y, direction::Type direction, u8 heights[6]);
    bool is_empty();
public:
    void drop_puyo(i8 x, cell::Type cell);
    void drop_pair(i8 x, direction::Type direction, cell::Pair pair);
    void drop_garbage(i32 count);
public:
    avec<Field, 19> pop();
public:
    void from(const char c[13][7]);
    void print();
};

inline i64 bench_pop(i32 iter)
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

    return time / iter;
};