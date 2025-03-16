#pragma once

#include <fstream>
#include <iomanip>
#include "../../../lib/nlohmann/json.hpp"
using json = nlohmann::json;

#include "node.h"
#include "form.h"
#include "quiet.h"

namespace beam
{

namespace eval
{

struct Weight
{
    i32 chain = 0;
    i32 y = 0;
    i32 key = 0;
    i32 chi = 0;

    i32 shape = 0;
    i32 well = 0;
    i32 bump = 0;
    i32 form = 0;
    i32 link_2 = 0;
    i32 link_3 = 0;
    i32 waste_14 = 0;
    i32 side = 0;
    i32 nuisance = 0;

    i32 tear = 0;
    i32 waste = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    chain,
    y,
    key,
    chi,
    shape,
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

void evaluate(node::Data& node, i32 tear, i32 waste, const Weight& w);

i32 get_chi(u8 heights[6], i8 x);

i32 get_shape(u8 heights[6]);

i32 get_well(u8 heights[6]);

i32 get_bump(u8 heights[6]);

i32 get_u(u8 heights[6]);

i32 get_link(Field& field);

std::pair<i32, i32> get_link_23(Field& field);

i32 get_waste_14(u8 row14);

};

};