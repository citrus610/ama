#pragma once

#include "../core/core.h"
#include "form.h"
#include "search/quiet.h"
#include <fstream>
#include <string>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

namespace Eval
{

struct Result
{
    i32 value = INT32_MIN;
    Field plan = Field();
    i32 q = 0;
};

struct Weight
{
    i32 chain = 0;
    i32 score = 0;
    i32 x = 0;
    i32 y = 0;
    i32 need = 0;
    i32 key = 0;
    i32 chi = 0;

    i32 dub_2 = 0;
    i32 dub_3 = 0;

    i32 shape = 0;
    i32 space14 = 0;
    i32 form = 0;
    i32 u = 0;
    i32 link = 0;
    i32 link_h = 0;
    i32 side = 0;
    i32 nuisance = 0;

    i32 tear = 0;
    i32 waste = 0;
};

constexpr Weight DEFAULT = {
    .chain = 1000,
    .score = 0,
    .x = 0,
    .y = 10,
    .need = 0,
    .key = 0,
    .chi = 0,

    .dub_2 = 0,
    .dub_3 = 0,

    .shape = -40,
    .space14 = 10,
    .form = 0,
    .u = -10,
    .link = 0,
    .link_h = 0,
    .side = 0,
    .nuisance = 0,

    .tear = -20,
    .waste = -10
};

Result evaluate(Field& field, i32 tear, i32 waste, Weight& w, i32 depth = 6, i32 drop = 3);

void get_shape(u8 heights[6], i32& shape_phase_min, i32& shape_phase_max);

i32 get_link(Field& field);

i32 get_u(u8 heights[6]);

i32 get_link_horizontal(Field& field);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    chain,
    score,
    x,
    y,
    need,
    key,
    chi,
    dub_2,
    dub_3,
    shape,
    space14,
    form,
    u,
    link,
    link_h,
    side,
    nuisance,
    tear,
    waste
)

};