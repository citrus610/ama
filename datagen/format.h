#pragma once

#include "../ai/ai.h"

namespace format
{

struct Data
{
    u16 field[5][6] = { 0 };
    i32 score = 0;
};

constexpr usize SIZE = sizeof(Data);

inline Data get_from_field(Field& field, i32 score)
{
    auto result = Data();

    for (i32 p = 0; p < 5; ++p) {
        alignas(16) u16 v[8];

        _mm_store_si128((__m128i*)v, field.data[p].data);

        for (i32 i = 0; i < 6; ++i) {
            result.field[p][i] = v[i];
        }
    }

    result.score = score;

    return result;
};

};