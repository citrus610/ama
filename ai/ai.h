#pragma once

#include "search/search.h"
#include "gaze.h"
#include "path.h"

namespace ai
{

constexpr i32 TRIGGER = 95000;

namespace style
{

enum class Attack
{
    NONE,
    WEAK,
    STRONG
};

enum class Defense
{
    WEAK,
    STRONG
};

struct Data
{
    Attack attack = Attack::STRONG;
    Defense defense = Defense::STRONG;
};

};

struct Action
{
    move::Placement placement = move::Placement();
    dfs::attack::Data attack = dfs::attack::Data();
};

struct Update
{
    std::optional<bool> form = {};
    std::optional<i32> trigger = {};
};

struct Result
{
    move::Placement placement = move::Placement();
    i32 eval = INT32_MIN;
    Update update = Update();
};

constexpr Result RESULT_DEFAULT = Result {
    .placement = move::Placement {
        .x = 2,
        .r = direction::Type::UP
    },
    .eval = INT32_MIN,
    .update = Update {
        .form = {},
        .trigger = {}
    }
};

Result build(
    Field field,
    cell::Queue queue,
    search::Result bsearch,
    search::Configs configs,
    dfs::attack::Result& asearch,
    search::Type type,
    i32 trigger = ai::TRIGGER,
    bool all_clear = true,
    bool stretch = true
);

Result think(
    gaze::Player self,
    gaze::Player enemy,
    search::Result bsearch,
    search::Configs configs,
    i32 target_point,
    style::Data style = style::Data(),
    i32 trigger = ai::TRIGGER,
    bool stretch = true
);

};