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

Field get_plan(Field& field, const Data& pattern);

constexpr Data LLR_1()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  6,  0,  0,  0 },
        { 1,  2,  3,  6,  0,  0 },
        { 1,  1,  2,  3,  6,  0 },
        { 3,  3,  2,  2,  5,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  1, -1, -1, -1,  0,  0 },
        { 0, -1,  1, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
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

constexpr Data LLR_2()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  6,  0,  0,  0 },
        { 1,  2,  3,  5,  0,  0 },
        { 1,  1,  2,  2,  5,  0 },
        { 3,  3,  2,  3,  6,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  1, -1, -1, -1,  0,  0 },
        { 0, -1,  1, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
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

constexpr Data GTR_1()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  6,  0,  0,  0 },
        { 1,  2,  3,  6,  0,  0 },
        { 1,  1,  2,  3,  6,  0 },
        { 2,  2,  3,  3,  6,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0,  0, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
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

constexpr Data GTR_2()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  6,  0,  0,  0 },
        { 1,  2,  3,  6,  6,  0 },
        { 1,  1,  2,  3,  3,  6 },
        { 2,  2,  3,  6,  6,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1,  0, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0,  0, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
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

constexpr Data SQR_1()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  0,  0,  0,  0 },
        { 1,  2,  6,  0,  6,  0 },
        { 1,  1,  3,  6,  3,  6 },
        { 2,  2,  2,  3,  3,  6 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 }
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

constexpr Data SQR_2()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  5,  6,  0,  0,  0 },
        { 1,  2,  3,  6,  0,  0 },
        { 1,  1,  3,  6,  6,  0 },
        { 2,  2,  2,  3,  3,  6 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 }
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


constexpr Data SUBM()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 3,  4,  0,  0,  0,  0 },
        { 1,  2,  4,  0,  0,  0 },
        { 1,  1,  5,  0,  0,  0 },
        { 2,  2,  2,  4,  0,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1,  0, -1,  0 },
        { 0, -1,  2,  0, -1, -1,  0 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0, -1, -1,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0,  0 }
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

constexpr Data SGTR()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  4,  5,  0,  0,  0 },
        { 1,  1,  3,  5,  0,  0 },
        { 1,  2,  2,  3,  5,  0 },
        { 2,  3,  3,  5,  0,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1, -1,  0,  0 },
        { 0, -1,  2, -1,  0,  0,  0 },
        { 0, -1, -1,  1,  0, -1,  0 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0,  0 }
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

constexpr Data MERI()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 4,  0,  0,  0,  0,  0 },
        { 1,  4,  6,  0,  0,  0 },
        { 1,  1,  3,  6,  0,  0 },
        { 2,  2,  2,  5,  0,  0 },
        { 3,  3,  3,  6,  0,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 }
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

constexpr Data FRON()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 4,  4,  6,  0,  0,  0 },
        { 1,  1,  3,  6,  0,  0 },
        { 1,  2,  2,  5,  0,  0 },
        { 3,  3,  2,  3,  6,  0 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  3, -1, -1, -1,  0,  0 },
        { 0, -1,  2, -1,  0, -1,  0 },
        { 0, -1, -1,  1,  0,  0, -1 },
        { 0, -1,  0,  0,  0,  0,  0 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0 }
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

constexpr Data YAYOI()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0 },
        { 5,  5,  5,  6,  6,  6 },
        { 2,  2,  2,  4,  4,  4 },
        { 1,  1,  1,  3,  3,  3 }
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 0,  0,  0,  0,  0,  0,  0 },
        { 0,  1, -1, -1,  0,  0,  0 },
        { 0, -1,  1,  0, -1, -1,  0 },
        { 0, -1,  0,  1, -1,  0,  0 },
        { 0,  0, -1, -1,  1,  0, -1 },
        { 0,  0, -1,  0,  0,  0,  0 },
        { 0,  0,  0,  0, -1,  0,  0 }
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

};