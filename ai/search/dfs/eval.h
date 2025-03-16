#pragma once

#include "quiet.h"
#include <fstream>
#include <string>
#include <iomanip>
#include "../../../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

namespace dfs
{

namespace eval
{

struct Result
{
    i32 value = INT32_MIN;
    i32 q = 0;
    Field plan = Field();
};

struct Weight
{
    i32 chain = 0;
    i32 y = 0;
    i32 key = 0;
    i32 chi = 0;

    i32 shape = 0;
    i32 well = 0;
    i32 bump = 0;
    i32 u = 0;
    i32 form = 0;
    i32 link_2 = 0;
    i32 link_3 = 0;
    i32 waste_14 = 0;
    i32 side = 0;
    i32 nuisance = 0;

    i32 tear = 0;
    i32 waste = 0;
};

constexpr Weight DEFAULT = {
    .chain = 500,
};

Result evaluate(Field& field, i32 tear, i32 waste, const Weight& w);

i32 get_static(Field& field, const Weight& w);

i32 get_chi(u8 heights[6], i8 x);

i32 get_shape(u8 heights[6], const i32 coef[6]);

i32 get_well(u8 heights[6]);

i32 get_bump(u8 heights[6]);

i32 get_u(u8 heights[6]);

i32 get_link(Field& field);

std::pair<i32, i32> get_link_23(Field& field);

i32 get_waste_14(u8 row14);

Field get_mobility(const Field& field);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    chain,
    y,
    key,
    chi,
    shape,
    u,
    well,
    bump,
    form,
    link_2,
    link_3,
    waste_14,
    side,
    nuisance,
    tear,
    waste
)

};

};