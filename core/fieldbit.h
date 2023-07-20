#pragma once

#include "def.h"
#include "cell.h"
#include "direction.h"

class FieldBit
{
public:
    __m128i data;
public:
    FieldBit();
public:
    bool operator == (const FieldBit& other);
    bool operator != (const FieldBit& other);
    FieldBit operator | (const FieldBit& other);
    FieldBit operator & (const FieldBit& other);
    FieldBit operator ^ (const FieldBit& other);
    FieldBit operator ~ ();
public:
    void set_bit(i8 x, i8 y);
    bool get_bit(i8 x, i8 y);
    u32 get_count();
    u16 get_col(i8 x);
    FieldBit get_expand();
    FieldBit get_mask_12();
    FieldBit get_mask_13();
    FieldBit get_mask_pop();
    FieldBit get_mask_group(i8 x, i8 y);
    FieldBit get_mask_group_4(i8 x, i8 y);
    FieldBit get_mask_group_lsb();
public:
    bool is_empty();
public:
    void pop(FieldBit& mask);
    void print();
};