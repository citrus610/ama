#pragma once

#include "search/dfs/attack.h"
#include "search/dfs/build.h"
#include "search/dfs/quiet.h"

namespace gaze
{

// Player's data
struct Player
{
    Field field = Field();
    cell::Queue queue = {};
    bool all_clear = false;
    i32 bonus = 0;
    i32 attack = 0;
    i32 attack_chain = 0;
    i32 attack_frame = 0;
    i32 dropping = 0;
};

// Gaze result
struct Data
{
    dfs::attack::Data main = dfs::attack::Data();
    dfs::attack::Data main_q = dfs::attack::Data();
    dfs::attack::Data harass = dfs::attack::Data();
    dfs::attack::Data early = dfs::attack::Data();
    dfs::attack::Data defence_1 = dfs::attack::Data();
    dfs::attack::Data defence_2 = dfs::attack::Data();
};

Data gaze(Field& field, dfs::attack::Result& asearch, i32 delay = 0);

i32 get_unburied_count(Field& field);

i32 get_accept_limit(Field& field);

i32 get_redundancy(Field& pre, Field& now);

i32 get_resource_balance(Field& field, Field& other);

bool is_garbage_obstruct(Field& field, chain::Score detect_highest);

bool is_small_field(Field& field, Field& other);

};