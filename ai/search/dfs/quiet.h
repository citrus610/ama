#pragma once

#include "../../../core/core.h"

namespace dfs
{

namespace quiet
{

struct Result
{
    chain::Score chain = { 0, 0 };
    i32 x = 0;
    Field plan = Field();
    Field remain = Field();
};

void search(
    Field& field,
    i32 depth,
    i32 drop,
    std::function<void(Result)> callback
);

void dfs(
    Field& field,
    Field& root,
    i8 x_ban,
    i32 pre_chain,
    i32 depth,
    std::function<void(Result)>& callback
);

std::pair<i8, i8> get_bound(u8 heights[6]);

bool is_reachable(u8 heights[6], i8 x);

void generate(
    Field& field,
    i8 x_min,
    i8 x_max,
    i8 x_ban,
    i32 drop,
    std::function<void(i8, i8, i8, i8)> callback
);

};

};