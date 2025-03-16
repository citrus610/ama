#pragma once

#include "eval.h"

namespace dfs
{

namespace attack
{

struct Data
{
    i32 count = 0;
    i32 score = 0;
    i32 score_total = 0;
    i32 frame = 0;
    i32 frame_real = 0;
    bool all_clear = false;
    i32 redundancy = INT32_MAX;
    i32 link = 0;
    Field parent = Field();
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
    move::Placement placement;
    attack::Data attack_max;
    std::vector<attack::Data> attacks;
    std::vector<attack::Data> attacks_ac;
    std::vector<attack::Data> attacks_detect;
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search(
    Field field,
    cell::Queue queue,
    bool detect = true,
    i32 frame_delay = 0,
    i32 thread_count = 4
);

void dfs(
    Node& node,
    cell::Queue& queue,
    Candidate& candidate,
    i32 depth,
    bool detect,
    i32 frame_delay
);

inline bool cmp_main(const attack::Data& a, const attack::Data& b)
{
    if (a.score != b.score) {
        return a.score < b.score;
    }

    return a.frame_real > b.frame_real;
};

inline bool cmp_main_enough(const attack::Data& a, const attack::Data& b, i32 trigger)
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

};