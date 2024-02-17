#pragma once

#include "../eval.h"
#include "../move.h"

namespace Build
{

enum Type
{
    BUILD,
    HARASS,
    SECOND_BIG,
    SECOND_SMALL,
    AC,
    COUNT
};

struct Node
{
    Field field;
    i32 tear = 0;
    i32 waste = 0;
};

struct Candidate
{
    Move::Placement placement;
    Eval::Result eval = Eval::Result();
    i32 eval_fast = INT32_MIN;
};

struct Result
{
    std::vector<Candidate> candidates;
};

class Thread
{
private:
    std::thread* thread;
    std::vector<Result> results;
public:
    bool search(Field field, Cell::Queue queue, std::vector<Eval::Weight> w);
    std::vector<Result> get();
    void clear();
};

Result search(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 thread_count = 4);

Eval::Result dfs(Node& node, std::vector<Cell::Pair>& queue, Eval::Weight& w, i32 depth);

};