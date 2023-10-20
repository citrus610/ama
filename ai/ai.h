#pragma once

#include "search.h"
#include "path.h"

#include <functional>

namespace AI
{

struct Result
{
    Move::Placement placement = Move::Placement();
    i32 eval = INT32_MIN;
};

constexpr Result RESULT_DEFAULT = Result {
    .placement = Move::Placement {
        .x = 2,
        .r = Direction::Type::UP
    },
    .eval = INT32_MIN
};

struct Enemy
{
    Field field = Field();
    std::vector<Cell::Pair> queue;
    i32 attack = 0;
    i32 attack_frame = 0;
    bool all_clear = false;
};

struct Data
{
    i32 target = 0;
    i32 bonus = 0;
    bool all_clear = false;
};

struct Weight
{
    Eval::Weight build = Eval::DEFAULT;
    Eval::Weight fast = Eval::FAST;
    Eval::Weight allclear = Eval::ALL_CLEAR;
};

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w = Eval::DEFAULT, i32 trigger = 78000);

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Weight w = Weight());

Result build(Search::Result& search, Field& field, i32 trigger, bool all_clear = true);

Result build_attack(
    Search::Result& search,
    Field& field,
    Data data,
    std::function<bool(Search::Attack&)> condition,
    std::function<bool(const std::pair<Move::Placement, Search::Attack>& a, const std::pair<Move::Placement, Search::Attack>& b)> cmp,
    bool all_clear = true
);

void get_candidate_eval(Search::Result& search, Eval::Weight w);

void get_attacks_eval(Search::Result& search, Eval::Weight w);

void iterate_candidates(Search::Result& search, std::function<void(Search::Candidate&)> func);

i32 get_unburied_count(Field& field);

bool get_garbage_obstruct(Field& field, Chain::Score& detect_highest, Chain::Score& detect_harass);

bool get_small_field(Field& field, Field& other);

bool get_bad_field(Field& field);

void get_gaze_field(Field& field, std::vector<Cell::Pair> queue, Chain::Score& detect_highest, Chain::Score& detect_harass);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    build,
    fast,
    allclear
)

};