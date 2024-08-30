#pragma once

#include "search/attack.h"
#include "search/build.h"
#include "search/quiet.h"

namespace Gaze
{

struct Player
{
    Field field = Field();
    Cell::Queue queue = {};
    bool all_clear = false;
    i32 bonus_point = 0;
    i32 attack = 0;
    i32 attack_frame = 0;
    i32 attack_chain = 0;
    i32 dropping = 0;
};

struct Data
{
    Attack::Data main = Attack::Data();
    Attack::Data main_fast = Attack::Data();
    std::vector<Attack::Data> harass;
    Attack::Data defence_1dub = Attack::Data();
    Attack::Data defence_2dub = Attack::Data();
    Attack::Data defence_3dub = Attack::Data();
};

Data gaze(Field& field, Attack::Result& asearch, i32 frame_offset = 0);

i32 get_unburied_count(Field& field);

i32 get_accept_limit(Field& field);

i32 get_redundancy(Field& pre, Field& now);

i32 get_resource_balance(Field& field, Field& other);

bool is_garbage_obstruct(Field& field, Chain::Score detect_highest);

bool is_small_field(Field& field, Field& other);

};