#pragma once

#include "def.h"

namespace direction
{

// Puyo pair's direction
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

    return 0;
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

    return 0;
};

constexpr Type get_rotate_cw(Type direction)
{
    switch (direction)
    {
    case Type::UP:
        return Type::RIGHT;
    case Type::RIGHT:
        return Type::DOWN;
    case Type::DOWN:
        return Type::LEFT;
    case Type::LEFT:
        return Type::UP;
    }

    return direction;
};

constexpr Type get_rotate_ccw(Type direction)
{
    switch (direction)
    {
    case Type::UP:
        return Type::LEFT;
    case Type::RIGHT:
        return Type::UP;
    case Type::DOWN:
        return Type::RIGHT;
    case Type::LEFT:
        return Type::DOWN;
    }

    return direction;
};

};