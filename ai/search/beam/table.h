#pragma once

#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#include "node.h"

// Allocate size bytes of uninitialized memory with specified alignment
// With size parameter be an integral multiple of alignment
inline void* malloc_aligned(usize alignment, usize size)
{
    void* ptr;

#if defined(__MINGW32__)
    ptr = _aligned_malloc(size, alignment);
#elif defined (__GNUC__)
    ptr = std::aligned_alloc(alignment, size);
#else
#error "Unsupported complier!"
#endif

    return ptr;
};

// Deallocates the memory previously allocated by malloc_aligned()
inline void free_aligned(void* ptr)
{
#if defined(__MINGW32__)
    _aligned_free(ptr);
#elif defined (__GNUC__)
    std::free(ptr);
#else
#error "Unsupported complier!"
#endif
};

// Table entry (8 bytes)
// - hash (2 bytes): the lower 16 bits of the position's hash
// - age (2 bytes): the age of the entry, we use age for entry replacement scheme, an entry is replacable/empty if it's age is different than the table's age
// - action (2 bytes): the action score that led to this position
// - eval (2 bytes): the evaluation value of the position
struct Entry
{
    u16 hash;
    u16 age;
    i16 action;
    i16 eval;
};

// Table buckets (32 bytes)
// Aligned on 32 bytes, storing 4 entries per bucket
struct alignas(32) Bucket
{
    Entry entries[4];
};

class Table
{
private:
    Bucket* buckets;
    u16 age;
    u64 count;
public:
    Table();
public:
    void resize(u64 kb = 256);
    void clear();
    void update();
public:
    u64 get_index(u64 hash);
    std::pair<bool, Entry*> get(u64 hash);
    void set(Entry* entry, u64 hash, i32 action, i32 eval);
};

static_assert(sizeof(Entry) == 8);
static_assert(sizeof(Bucket) == 32);