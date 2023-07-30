#pragma once

#include "../core/core.h"
#include "detect.h"

namespace Form
{

constexpr u8 HEIGHT = 6;
constexpr u8 AREA = HEIGHT * 6;

struct Data
{
    u8 form[HEIGHT][6] = { 0 };
    i8 matrix[AREA][AREA] = { 0 };
};

i32 evaluate(Field& field, u8 height[6], const Data& pattern);

constexpr Data DEFAULT()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  0,  0,  0,  0,  0,  0 },
        {  4,  5,  0,  0,  0,  0 },
        {  1,  2,  3,  0,  0,  0 },
        {  1,  1,  2,  3,  0,  0 },
        {  2,  2,  3,  0,  0,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0 },
        { 0, -1,  2, -1,  0, -1 },
        { 0,  0, -1,  0,  0,  0 },
        { 0, -1,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR1()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  0,  0,  0,  0,  0,  0 },
        {  4,  6,  0,  0,  0,  0 },
        {  1,  2,  3,  5,  5,  5 },
        {  1,  1,  2,  3,  3,  0 },
        {  2,  2,  3,  5,  5,  5 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1, -1,  0, -1 },
        { 0,  0, -1,  1, -1, -1,  0 },
        { 0, -1, -1, -1,  3,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR2()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  0,  0,  0,  0,  0,  0 },
        {  4,  6,  0,  0,  0,  0 },
        {  1,  2,  3,  5,  0,  0 },
        {  1,  1,  2,  3,  5,  0 },
        {  2,  2,  3,  3,  5,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1, -1,  0, -1 },
        { 0,  0, -1,  1, -1, -1,  0 },
        { 0, -1, -1, -1,  3,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR3()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  6,  6,  0,  0,  0,  0 },
        {  4,  4,  6,  5,  0,  0 },
        {  1,  2,  3,  3,  5,  0 },
        {  1,  1,  2,  3,  5,  0 },
        {  2,  2,  3,  5,  0,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1, -1,  0,  0 },
        { 0,  0, -1,  1, -1, -1,  0 },
        { 0, -1, -1, -1,  1,  0, -1 },
        { 0,  0,  0, -1,  0,  0,  0 },
        { 0,  0,  0,  0, -1,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR4()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  6,  6,  0,  0,  0,  0 },
        {  4,  4,  6,  0,  0,  0 },
        {  1,  2,  3,  5,  0,  0 },
        {  1,  1,  2,  5,  5,  0 },
        {  2,  2,  3,  3,  3,  5 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1, -1,  0,  0 },
        { 0,  0, -1,  1, -1, -1, -1 },
        { 0, -1, -1, -1,  1,  0, -1 },
        { 0,  0,  0, -1,  0,  0,  0 },
        { 0,  0,  0, -1, -1,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR5()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  0,  0,  0,  0,  0,  0 },
        {  6,  6,  0,  0,  0,  0 },
        {  4,  4,  6,  0,  0,  0 },
        {  1,  2,  3,  5,  5,  5 },
        {  1,  1,  2,  3,  3,  3 },
        {  2,  2,  5,  5,  5,  5 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1, -1, -1,  0 },
        { 0,  0, -1,  1, -1, -1, -1 },
        { 0, -1, -1, -1,  1,  0, -1 },
        { 0,  0, -1, -1,  0,  0,  0 },
        { 0,  0,  0, -1, -1,  0,  0 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

constexpr Data GTR[5] = { GTR1(), GTR2(), GTR3(), GTR4(), GTR5() };

};