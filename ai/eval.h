#pragma once

#include "../core/core.h"
#include "detect.h"
#include "form.h"
#include <fstream>
#include <string>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

namespace Eval
{

struct Weight
{
    i32 shape = 0;
    i32 shape_coef_hi[6] = { 0, 0, 0, 0, 0, 0 };
    i32 shape_coef_lo[6] = { 0, 0, 0, 0, 0, 0 };
    i32 nuisance = 0;
    i32 side_bias = 0;
    i32 link_hor_2 = 0;
    i32 link_hor_3 = 0;
    i32 link_ver_2 = 0;
    i32 link_ver_3 = 0;

    i32 form = 0;

    i32 tear = 0;
    i32 waste = 0;

    i32 chain_count = 0;
    i32 chain_score = 0;
    i32 chain_height = 0;
    i32 chain_needed = 0;
    i32 chain_x[6] = { 0, 0, 0, 0, 0, 0 };
};

i32 evaluate(Field& field, i32 tear, i32 waste, Weight& w);

constexpr Weight DEFAULT_WEIGHT = {
    .shape = -150,
    .shape_coef_hi = { 4, 2, 0, 0, 0, 2 },
    .shape_coef_lo = { 4, 2, 0, 0, 0, 2 },
    .nuisance = -100,
    .side_bias = 0,
    .link_hor_2 = 100,
    .link_hor_3 = 150,
    .link_ver_2 = 100,
    .link_ver_3 = 0,

    .form = 100,

    .tear = -250,
    .waste = -200,

    .chain_count = 1000,
    .chain_score = 10,
    .chain_height = 200,
    .chain_needed = 0,
    .chain_x = { 0, 200, 100, -100, -200, -300 },
};

constexpr Weight FAST_WEIGHT = {
    .shape = 0,
    .shape_coef_hi = { 0, 0, 0, 0, 0, 0 },
    .shape_coef_lo = { 0, 0, 0, 0, 0, 0 },
    .nuisance = -100,
    .side_bias = -100,
    .link_hor_2 = 10,
    .link_hor_3 = 20,
    .link_ver_2 = 50,
    .link_ver_3 = 100,

    .form = 0,

    .tear = -250,
    .waste = -200,

    .chain_count = 1000,
    .chain_score = 10,
    .chain_height = 200,
    .chain_needed = 0,
    .chain_x = { 0, 0, 0, 0, 0, 0 },
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    shape,
    shape_coef_hi[0],
    shape_coef_hi[1],
    shape_coef_hi[2],
    shape_coef_hi[3],
    shape_coef_hi[4],
    shape_coef_hi[5],
    shape_coef_lo[0],
    shape_coef_lo[1],
    shape_coef_lo[2],
    shape_coef_lo[3],
    shape_coef_lo[4],
    shape_coef_lo[5],
    nuisance,
    side_bias,
    link_hor_2,
    link_hor_3,
    link_ver_2,
    link_ver_3,
    form,
    tear,
    waste,
    chain_count,
    chain_score,
    chain_height,
    chain_needed,
    chain_x[0],
    chain_x[1],
    chain_x[2],
    chain_x[3],
    chain_x[4],
    chain_x[5]
)

};