#pragma once

#include "layer.h"
#include "eval.h"

namespace beam
{

constexpr size_t BRANCH = 6;
constexpr size_t PRUNE = 5000;

struct Configs
{
    size_t width = 250;
    size_t depth = 16;
    size_t trigger = 100000;
};

struct Candidate
{
    move::Placement placement = move::Placement();
    size_t score = 0;
};

struct Result
{
    std::vector<Candidate> candidates;
};

void expand(
    const cell::Pair& pair,
    node::Data& node,
    const eval::Weight& w,
    std::function<void(node::Data&, const move::Placement&, const chain::Score&)> callback
);

void think(
    const cell::Pair& pair,
    std::vector<Candidate>& candidates,
    Layer& parents,
    Layer& children,
    const eval::Weight& w
);

Result search(
    Field field,
    cell::Queue queue,
    eval::Weight w,
    Configs configs = Configs()
);

Result search_multi(
    Field field,
    cell::Queue queue,
    eval::Weight w,
    Configs configs = Configs()
);

cell::Queue get_queue_random(i32 id, size_t count);

inline bool operator < (const Candidate& a, const Candidate& b)
{
    return a.score < b.score;
};

};