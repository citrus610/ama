#pragma once

#include "../core/core.h"

namespace Detect
{

struct Score
{
    Chain::Score chain = { 0, 0 };
    i32 needed = 0;
    i32 height = 0;
    i32 x = 0;
};

struct Result
{
    Score score = Score();
    Field plan = Field();
};

void detect(Field& field, std::function<void(Result)> callback, i32 drop_max = 2, i32 drop_min = 1);

Result detect_fast(Field& field);

bool is_well(u8 heights[6], i8 x);

bool is_reachable(Field& field, u8 heights[6], i8 x, u8 added, u8 p);

static inline bool cmp_main(Result a, Result b)
{
    if (a.score.chain.count != b.score.chain.count) {
        return a.score.chain.count < b.score.chain.count;
    }

    if (a.score.height != b.score.height) {
        return a.score.height < b.score.height;
    }
    
    // return a.plan.get_count() > b.plan.get_count();
    return a.score.chain.score < b.score.chain.score;
};

};