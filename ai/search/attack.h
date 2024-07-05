#pragma once

#include "../../core/core.h"
#include "../move.h"
#include "../eval.h"

namespace Attack
{

struct Data
{
    i32 count = 0;
    i32 score = 0;
    i32 score_total = 0;
    i32 frame = 0;
    i32 frame_real = 0;
    i32 redundancy = INT32_MAX;
    bool all_clear = false;
    Field result = Field();
};

struct Node
{
    Field field;
    i32 score = 0;
    i32 frame = 0;
};

struct Candidate
{
    Move::Placement placement;
    Attack::Data attack_max;
    std::vector<Attack::Data> attacks;
    std::vector<Attack::Data> attacks_ac;
    std::vector<Attack::Data> attacks_detect;
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search(
    Field field,
    std::vector<Cell::Pair> queue,
    bool deep = true,
    i32 frame_delay = 0,
    i32 thread_count = 4
);

void dfs(
    Node& node,
    std::vector<Cell::Pair>& queue,
    Candidate& candidate,
    i32 depth,
    bool deep,
    i32 frame_delay
);

static bool cmp_main(const Attack::Data& a, const Attack::Data& b)
{
    if (a.score != b.score) {
        return a.score < b.score;
    }

    return a.frame_real > b.frame_real;
};

static bool cmp_main_enough(const Attack::Data& a, const Attack::Data& b, i32 trigger)
{
    bool a_over = a.score >= trigger;
    bool b_over = b.score >= trigger;

    if (a_over != b_over) {
        return a_over < b_over;
    }

    if (a.frame_real != b.frame_real) {
        return a.frame_real > b.frame_real;
    }

    return a.score < b.score;
};

};