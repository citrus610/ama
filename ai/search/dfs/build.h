#pragma once

#include "eval.h"

namespace dfs
{

namespace build
{

struct Node
{
    Field field;
    i32 tear = 0;
    i32 waste = 0;
};

struct Candidate
{
    move::Placement placement;
    eval::Result eval = eval::Result();
    i32 eval_fast = INT32_MIN;
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search(Field field, cell::Queue queue, eval::Weight w, i32 thread_count = 4);

eval::Result dfs(Node& node, cell::Queue& queue, eval::Weight& w, i32 depth);

};

};