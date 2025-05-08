#pragma once

#include "def.h"

namespace cell
{

// Puyo cell type
enum class Type : u8
{
    RED,
    YELLOW,
    GREEN,
    BLUE,
    GARBAGE,
    NONE
};

// Puyo pair
typedef std::pair<Type, Type> Pair;

// Puyo queue
typedef std::vector<Pair> Queue;

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

// Create puyo queue based on Puyo eSport's algorithm
inline Queue create_queue(u32 seed)
{
    // Puyo eSport's random number generation algorithm
    auto rng = [&] () -> u32 {
        seed = (seed * u32(0x5D588B65) + u32(0x269EC3)) & u32(0xFFFFFFFF);
        return seed;
    };

    // Calls rng() 5 times
    // In Puyo eSport, rng() is called 5 times to choose the colors in the queue
    for (i32 i = 0; i < 5; ++i) {
        rng();
    }

    // Puyo eSport creates 3 queues everytime when initializing:
    // - a 3-color queue
    // - a 4-color queue
    // - a 5-color queue
    u8 queue[3][256] = { 0 };

    // Initializes all 3 queues
    for (i32 mode = 0; mode < 3; ++mode) {
        for (i32 i = 0; i < 256; ++i) {
            queue[mode][i] = i % (mode + 3);
        }
    }

    // Shuffles all 3 queues
    for (i32 mode = 0; mode < 3; ++mode) {
        // 1st shuffle
        for (i32 col = 0; col < 15; ++col) {
            for (i32 i = 0; i < 8; ++i) {
                i32 n1 = (rng() >> 28) + col * 16;
                i32 n2 = (rng() >> 28) + (col + 1) * 16;

                std::swap(queue[mode][n1], queue[mode][n2]);
            }
        }

        // 2nd shuffle
        for (i32 col = 0; col < 7; ++col) {
            for (i32 i = 0; i < 16; ++i) {
                i32 n1 = (rng() >> 27) + col * 32;
                i32 n2 = (rng() >> 27) + (col + 1) * 32;

                std::swap(queue[mode][n1], queue[mode][n2]);
            }
        }

        // 3rd shuffle
        for (i32 col = 0; col < 3; ++col) {
            for (i32 i = 0; i < 32; ++i) {
                i32 n1 = (rng() >> 26) + col * 64;
                i32 n2 = (rng() >> 26) + (col + 1) * 64;

                std::swap(queue[mode][n1], queue[mode][n2]);
            }
        }
    }

    // Replace the first 2 pairs of the 4-color and 5-color queues with the first 2 pairs of the 3-color queue
    // This ensures you'll only get 3 different colors in your first 2 pairs
    for (i32 i = 0; i < 4; ++i) {
        queue[1][i] = queue[0][i];
        queue[2][i] = queue[0][i];
    }

    std::vector<cell::Pair> result;

    for (i32 i = 0; i < 128; ++i) {
        result.push_back({ cell::Type(queue[1][i * 2]), cell::Type(queue[1][i * 2 + 1]) });
    }

    return result;
};

};