#pragma once

#include "search/search.h"
#include "path.h"

namespace ai
{

constexpr i32 TRIGGER = 90000;

struct Result
{
    move::Placement placement = move::Placement();
    std::optional<Field> plan = std::nullopt;
    i32 eval = INT32_MIN;
};

constexpr Result RESULT_DEFAULT = Result {
    .placement = move::Placement {
        .x = 2,
        .r = direction::Type::UP
    },
    .plan = std::nullopt,
    .eval = INT32_MIN
};

};