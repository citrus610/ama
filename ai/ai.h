#pragma once

#include "search/build.h"
#include "search/attack.h"
#include "gaze.h"
#include "path.h"

#include <functional>

namespace AI
{

constexpr i32 TRIGGER_DEFAULT = 78000;

struct Result
{
    Move::Placement placement = Move::Placement();
    std::optional<Field> plan = std::nullopt;
    i32 eval = INT32_MIN;
};

constexpr Result RESULT_DEFAULT = Result {
    .placement = Move::Placement {
        .x = 2,
        .r = Direction::Type::UP
    },
    .plan = std::nullopt,
    .eval = INT32_MIN
};

Result think_1p(Field field, Cell::Queue queue, Eval::Weight w = Eval::DEFAULT, bool all_clear = true, i32 trigger = TRIGGER_DEFAULT);

Result build(Build::Result& bsearch, Attack::Result& asearch, bool all_clear = true, i32 trigger = TRIGGER_DEFAULT);

Result think_2p(Gaze::Player self, Gaze::Player enemy, Attack::Result& asearch, std::vector<Build::Result>& bsearch, Eval::Weight w[], i32 target_point);

};