#pragma once

#include "../../../core/core.h"

namespace beam
{

namespace quiet
{

struct Result
{
    chain::Score chain = { 0, 0 };
    i32 x = 0;
    i32 key = 0;
    Field remain = Field();
};

void search(
    Field& field,
    i32 drop,
    std::function<void(Result)> callback
);

void generate(
    Field& field,
    i8 x_min,
    i8 x_max,
    i32 drop,
    std::function<void(i8, i8, i8)> callback
);

std::pair<i8, i8> get_bound(u8 heights[6]);

};

};