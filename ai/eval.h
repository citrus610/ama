#pragma once

#include "../core/core.h"
#include "qsearch.h"
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
    i32 chain = 0;
    i32 score = 0;
    i32 y = 0;
    i32 need = 0;
    i32 well = 0;
    i32 extendable = 0;

    i32 shape = 0;
    i32 u = 0;
    i32 form = 0;
    i32 link_top = 0;
    i32 link_h = 0;
    i32 link_v = 0;
    i32 link_no = 0;
    i32 side = 0;
    i32 nuisance = 0;

    i32 tear = 0;
    i32 waste = 0;
};

constexpr Weight DEFAULT = {
    .chain = 500,
    .score = 0,
    .y = 150,
    .need = -250,
    .well = -150,
    .extendable = 100,

    .shape = -80,
    .u = -75,
    .form = 100,
    .link_top = 0,
    .link_h = 0,
    .link_v = 0,
    .link_no = -100,
    .side = 0,
    .nuisance = -100,

    .tear = -75,
    .waste = -150
};

constexpr Weight FAST = {
    .chain = 500,
    .score = 0,
    .y = 57,
    .need = -257,
    .well = -148,
    .extendable = 91,

    .shape = 0,
    .u = 0,
    .form = 0,
    .link_top = 0,
    .link_h = 0,
    .link_v = 0,
    .link_no = -61,
    .side = 0,
    .nuisance = -100,

    .tear = -71,
    .waste = -33
};

constexpr Weight ALL_CLEAR = {
    .chain = 500,
    .score = 0,
    .y = 57,
    .need = -257,
    .well = -148,
    .extendable = 91,

    .shape = 0,
    .u = 0,
    .form = 0,
    .link_top = 0,
    .link_h = 0,
    .link_v = 0,
    .link_no = -61,
    .side = 100,
    .nuisance = -100,

    .tear = -71,
    .waste = -33
};

i32 evaluate(Field& field, i32 tear, i32 waste, i32 depth, Weight& w);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    chain,
    score,
    y,
    need,
    well,
    extendable,
    shape,
    u,
    form,
    link_top,
    link_h,
    link_v,
    link_no,
    side,
    nuisance,
    tear,
    waste
)

};