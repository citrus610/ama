#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <array>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <bitset>
#include <bit>
#include <thread>
#include <limits>
#include <mutex>
#include <atomic>
#include <optional>
#include <x86intrin.h>
#include <condition_variable>
#include <numeric>
#include <stdalign.h>
#include <functional>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

inline u16 pext16(u16 input, u16 mask)
{
#ifdef PEXT
    return _pext_u32(u32(input), u32(mask));
#else
    u16 result = 0;

    for (u16 bb = 1; mask != 0; bb += bb) {
        if (input & mask & -mask) {
            result |= bb;
        }
        
        mask &= (mask - 1);
    }

    return result;
#endif
};