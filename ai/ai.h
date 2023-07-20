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

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w = Eval::DEFAULT_WEIGHT, i32 trigger_score = 71000);

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w = Eval::DEFAULT_WEIGHT);

Result build(Search::Result& search_result, Field& field, i32 trigger_score);

Result build_attack(Search::Result& search_result, Field& field, Data data, std::function<bool(Search::Attack&)> condition);

void get_candidate_eval(Search::Result& search_result, Eval::Weight w);

void get_attacks_eval(Search::Result& search_result, Eval::Weight w);

std::vector<std::pair<Move::Placement, Search::Attack>> get_attacks_with_condition(Search::Result& search_result, std::function<bool(Search::Attack&)> condition);

i32 get_unburied_count(Field& field);

bool get_garbage_obstruct(Field& field, std::vector<Cell::Pair>& queue);

bool get_small_field(Field& field, Field& other);

bool get_in_danger(Field& field);

bool get_can_receive_garbage(Field& field, Enemy& enemy);

};