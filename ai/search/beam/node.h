#pragma once

#include "../../../core/core.h"
#include "../../../lib/rapidhash/rapidhash.h"

namespace beam
{

namespace node
{

struct Score
{
    i32 eval = 0;
    i32 action = 0;
};

struct Data
{
    Field field = Field();
    Score score = Score();
    i32 index = -1;
};

inline bool operator < (const Score& a, const Score& b)
{
    return a.action + a.eval < b.action + b.eval;
};

inline bool operator < (const node::Data& a, const node::Data& b)
{
    return a.score < b.score;
};

inline u64 get_hash(const Data& node)
{
    return rapidhash((const void*)node.field.data, 80);
};

};

};