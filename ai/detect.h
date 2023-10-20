#pragma once

#include "../core/core.h"

namespace Detect
{

struct Result
{
    i32 chain = 0;
    i32 score = 0;
    i32 y = 0;
    i32 extensibility = 0;
    Field plan;
};

void detect(Field& field, std::function<void(Result)> callback, i32 drop = 2, i32 drop_well = 1);

void detect_fast(Field& field, std::function<void(Result)> callback);

bool is_well(u8 heights[6], i8 x);

};