#pragma once

#include "../core/core.h"

namespace Move
{

enum class Type
{
    LEFT,
    RIGHT,
    CW,
    CCW,
    M180,
    DROP,
    NONE
};

struct Placement
{
    i8 x = 0;
    Direction::Type r = Direction::Type::UP;
};

avec<Placement, 22> generate(Field& field, bool pair_equal);

bool is_valid(u8 heights[6], i8 x, Direction::Type r);

constexpr char to_char(Type cell)
{
    switch (cell)
    {
    case Type::LEFT:
        return 'L';
    case Type::RIGHT:
        return 'R';
    case Type::CW:
        return 'W';
    case Type::CCW:
        return 'C';
    case Type::M180:
        return 'M';
    case Type::DROP:
        return 'D';
    case Type::NONE:
        return ' ';
    }
    return ' ';
};

constexpr Type from_char(char c)
{
    switch (c)
    {
    case 'L':
        return Type::LEFT;
    case 'R':
        return Type::RIGHT;
    case 'W':
        return Type::CW;
    case 'C':
        return Type::CCW;
    case 'M':
        return Type::M180;
    case 'D':
        return Type::DROP;
    default:
        return Type::NONE;
    }
    return Type::NONE;
};

constexpr std::string to_str(Type cell)
{
    switch (cell)
    {
    case Type::LEFT:
        return "LEFT";
    case Type::RIGHT:
        return "RIGHT";
    case Type::CW:
        return "CW";
    case Type::CCW:
        return "CCW";
    case Type::M180:
        return "M180";
    case Type::DROP:
        return "DROP";
    case Type::NONE:
        return "_";
    }
    return "";
};

static i64 bench_move(i32 iter)
{
    const char c1[13][7] = {
        "......",
        "....#.", 
        "#...#.", 
        "....#.", 
        "....#.", 
        "######",
        "######",
        "######",
        "######",
        "######",
        "######",
        "######",
        "######"
    };
    auto f = Field();
    f.from(c1);
    f.print();

    i64 time_1 = 0;
    avec<Placement, 22> plc1 = avec<Placement, 22>();

    for (i32 i = 0; i < iter; ++i) {
        auto time_start = std::chrono::high_resolution_clock::now();
        plc1 = generate(f, false);
        auto time_end = std::chrono::high_resolution_clock::now();
        time_1 += std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    }

    for (i32 i = 0; i < plc1.get_size(); ++i) {
        printf("x: %d, r: %d\n", plc1[i].x, static_cast<i32>(plc1[i].r));
    }

    std::cout << "time1: " << int(time_1 / iter) << "\n";

    return 0;
};

};