#pragma once

#include "def.h"

namespace Cell
{

enum class Type : u8
{
    RED,
    YELLOW,
    GREEN,
    BLUE,
    GARBAGE,
    NONE
};

typedef std::pair<Type, Type> Pair;

constexpr u8 COUNT = 5;

constexpr char to_char(Type cell)
{
    switch (cell)
    {
    case Type::RED:
        return 'R';
    case Type::YELLOW:
        return 'Y';
    case Type::GREEN:
        return 'G';
    case Type::BLUE:
        return 'B';
    case Type::GARBAGE:
        return '#';
    case Type::NONE:
        return '.';
    }
    return '.';
};

constexpr Type from_char(char c)
{
    switch (c)
    {
    case 'R':
        return Type::RED;
    case 'Y':
        return Type::YELLOW;
    case 'G':
        return Type::GREEN;
    case 'B':
        return Type::BLUE;
    case '#':
        return Type::GARBAGE;
    default:
        return Type::NONE;
    }
    return Type::NONE;
};

};