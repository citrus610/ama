#pragma once

#include "../../core/core.h"

namespace Quiet
{

struct Result
{
    i32 chain = 0; // chain count
    i32 score = 0; // chain score
    i32 x = 0; // ignition x
    i32 depth = 0; // depth of the quiet search
    Field plan; // field before triggering
    Field remain; // field after triggering
};

void search(
    Field& field,
    i32 depth,
    i32 drop,
    std::function<void(Result)> callback
);

void dfs(
    Field& field,
    Field& pre,
    i8 x_min,
    i8 x_max,
    i8 x_ban,
    i32 pre_count,
    i32 depth,
    std::function<void(Result)>& callback
);

void generate(
    Field& field,
    i8 x_min,
    i8 x_max,
    i8 x_ban,
    i32 drop,
    std::function<void(i8, i8, i32)> callback
);

i32 get_redundancy();

};