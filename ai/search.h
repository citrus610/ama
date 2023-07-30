#pragma once

#include "detect.h"
#include "move.h"
#include "eval.h"

namespace Search
{

struct Attack
{
    i32 score = 0;
    i32 count = 0;
    i32 frame = 0;
    i32 score_total = 0;
    bool all_clear = false;
    Field result;
    i32 eval = INT32_MIN;
};

struct Node
{
    Field field;
    i32 score = 0;
    i32 frame = 0;
    i32 tear = 0;
    i32 waste = 0;
};

struct Candidate
{
    Move::Placement placement;
    std::vector<Attack> attacks;
    std::vector<Attack> attacks_detect;
    std::vector<Node> plans;
    Node plan_fast;
    i32 eval = INT32_MIN;
    i32 eval_fast = INT32_MIN;
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search(
    Field field,
    std::vector<Cell::Pair> queue,
    i32 thread_count = 4
);

void dfs(
    Node& node,
    std::vector<Cell::Pair>& queue,
    std::vector<Attack>& attacks,
    std::vector<Attack>& attacks_detect,
    std::vector<Node>& plans,
    i32 depth
);

};