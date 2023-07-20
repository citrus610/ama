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
    i32 u = 0;
    i32 bump_mid = 0;
    i32 nuisance = 0;
    i32 side_bias = 0;
    i32 link_hor_2 = 0;
    i32 link_hor_3 = 0;
    i32 link_ver_2 = 0;
    i32 link_ver_3 = 0;
    i32 link_mid = 0;
    i32 form = 0;

    i32 tear = 0;

    i32 chain_count = 0;
    i32 chain_score = 0;
    i32 chain_height = 0;
    i32 chain_needed = 0;
    i32 chain_x = 0;
};

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 tear, Weight& w);

constexpr Weight DEFAULT_WEIGHT = {
    .u = -150,
    .bump_mid = 0,
    .nuisance = -100,
    .side_bias = 0,
    .link_hor_2 = 50,
    .link_hor_3 = 100,
    .link_ver_2 = 50,
    .link_ver_3 = 0,
    .link_mid = 0,
    .form = 0,

    .tear = -200,

    .chain_count = 1000,
    .chain_score = 10,
    .chain_height = 200,
    .chain_needed = 0,
    .chain_x = 100,
};

constexpr Weight FAST_WEIGHT = {
    .u = 0,
    .bump_mid = 0,
    .nuisance = -100,
    .side_bias = -100,
    .link_hor_2 = 10,
    .link_hor_3 = 20,
    .link_ver_2 = 50,
    .link_ver_3 = 100,
    .link_mid = 0,
    .form = 0,

    .tear = -250,

    .chain_count = 1000,
    .chain_score = 10,
    .chain_height = 200,
    .chain_needed = 0,
    .chain_x = 0
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    u,
    bump_mid,
    nuisance,
    side_bias,
    link_hor_2,
    link_hor_3,
    link_ver_2,
    link_ver_3,
    link_mid,
    form,
    tear,
    chain_count,
    chain_score,
    chain_height,
    chain_needed,
    chain_x
)

};