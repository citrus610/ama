#pragma once

#include "../core/core.h"

namespace Move
{

struct Placement
{
    i8 x = 0;
    Direction::Type r = Direction::Type::UP;
};

avec<Placement, 22> generate(Field& field, bool pair_equal);

bool is_valid(u8 heights[6], u8 row14, i8 x, Direction::Type r);

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