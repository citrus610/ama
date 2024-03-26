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
    i32 y = 0;
    i32 need = 0;
    i32 key = 0;
    i32 key_s = 0;
    i32 chi = 0;

    i32 dub_2 = 0;
    i32 dub_3 = 0;

    i32 shape = 0;
    i32 u = 0;
    i32 form = 0;
    i32 link_2 = 0;
    i32 link_3 = 0;
    i32 link_h = 0;
    i32 waste_14 = 0;
    i32 side = 0;
    i32 nuisance = 0;

    i32 tear = 0;
    i32 waste = 0;
};

constexpr Weight DEFAULT = {
    .chain = 500,
    .score = 0,
    .y = 147,
    .need = -7,
    .key = -282,
    .chi = 117,

    .dub_2 = 0,
    .dub_3 = 0,

    .shape = -33,
    .u = -100,
    .form = 100,
    .side = 0,
    .nuisance = -200,

    .tear = -233,
    .waste = -238
};

Result evaluate(Field& field, i32 tear, i32 waste, Weight& w);

i32 get_shape(u8 heights[6]);

i32 get_u(u8 heights[6]);

void get_link(Field& field, i32& link_2, i32& link_3);

i32 get_link_horizontal(Field& field);

i32 get_waste_14(u8 row14);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    chain,
    score,
    y,
    need,
    key,
    key_s,
    chi,
    dub_2,
    dub_3,
    shape,
    u,
    form,
    link_2,
    link_3,
    link_h,
    waste_14,
    side,
    nuisance,
    tear,
    waste
)

};