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

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i128 = __int128;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = unsigned __int128;

using f32 = float;
using f64 = double;

using usize = size_t;

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