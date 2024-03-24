#pragma once

#include "../eval.h"
#include "../move.h"

namespace Beam
{

struct Node
{
    Field field = Field();
    i32 index = -1;
    i32 eval = INT32_MIN;
    i32 tear = 0;
    i32 waste = 0;
};

class Layer
{
public:
    std::vector<Node> data;
    size_t width;
public:
    Layer();
public:
    void init(size_t width);
    void clear();
    void add(Node& node);
    void sort();
};

struct Candidate
{
    Move::Placement placement;
    i32 score = 0;
};

struct Result
{
    std::vector<Candidate> candidates;
};

struct Score
{
    i32 index = -1;
    i32 score = 0;
};

Result search(
    Field field,
    Cell::Queue queue,
    Eval::Weight w = Eval::DEFAULT,
    i32 trigger = 80000,
    size_t width = 64,
    size_t thread = 4
);

Score expand(
    Node& node,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight& w
);

Score think(
    Cell::Pair pair,
    Layer& old_layer,
    Layer& new_layer,
    Eval::Weight& w
);

Score init_candidates(
    std::vector<Candidate>& candidates,
    Node& root,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight& w
);

Cell::Queue generate_queue(size_t size, size_t seed);

};