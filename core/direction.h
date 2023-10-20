#pragma once

#include "def.h"

namespace Direction
{

enum class Type : u8
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

constexpr u32 COUNT = 4;

constexpr i8 get_offset_x(Type direction)
{
    switch (direction)
    {
    case Type::UP:
        return 0;
    case Type::RIGHT:
        return 1;
    case Type::DOWN:
        return 0;
    case Type::LEFT:
        return -1;
    }
};

constexpr i8 get_offset_y(Type direction)
{
    switch (direction)
    {
    case Type::UP:
        return 1;
    case Type::RIGHT:
        return 0;
    case Type::DOWN:
        return -1;
    case Type::LEFT:
        return 0;
    }
};

};