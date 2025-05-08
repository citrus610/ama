#include "fieldbit.h"

FieldBit::FieldBit()
{
    this->data = _mm_setzero_si128();
};

bool FieldBit::operator == (const FieldBit& other)
{
    __m128i neq = _mm_xor_si128(this->data, other.data);
    return _mm_test_all_zeros(neq, neq);
};

bool FieldBit::operator != (const FieldBit& other)
{
    __m128i neq = _mm_xor_si128(this->data, other.data);
    return !_mm_test_all_zeros(neq, neq);
};

FieldBit FieldBit::operator | (const FieldBit& other)
{
    FieldBit result;
    result.data = this->data | other.data;
    return result;
};

FieldBit FieldBit::operator & (const FieldBit& other)
{
    FieldBit result;
    result.data = this->data & other.data;
    return result;
};

FieldBit FieldBit::operator ^ (const FieldBit& other)
{
    FieldBit result;
    result.data = this->data ^ other.data;
    return result;
};

FieldBit FieldBit::operator ~ ()
{
    FieldBit result;
    result.data = ~this->data;
    return result;
};

// Turns on a bit
void FieldBit::set_bit(i8 x, i8 y)
{
    assert(x >= 0 && x < 6);

    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);

    v[x] |= 1 << y;

    this->data = _mm_load_si128((const __m128i*)v);
};

// Checks if a bit is set
bool FieldBit::get_bit(i8 x, i8 y)
{
    if (x < 0 || x > 5 || y < 0 || y > 12) {
        return false;
    }

    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);

    return v[x] & (1 << y);
};

// Returns the number of set bits
u32 FieldBit::get_count()
{
    alignas(16) u64 v[2];
    _mm_store_si128((__m128i*)v, this->data);

    return std::popcount(v[0]) + std::popcount(v[1]);
};

// Returns a column of the bitfield
u16 FieldBit::get_col(i8 x)
{
    assert(x >= 0 && x < 6);

    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);

    return v[x];
};

// Expands a bitfield and returns the result
// Ex:
    // ......      ......
    // ......      .XX...
    // .XX...  ->  XXXX..
    // ......      .XX..X
    // .....X      ....XX
FieldBit FieldBit::get_expand()
{
    __m128i r = _mm_srli_si128(this->data, 2);
    __m128i l = _mm_slli_si128(this->data, 2);
    __m128i u = _mm_srli_epi16(this->data, 1);
    __m128i d = _mm_slli_epi16(this->data, 1);

    auto result = *this;
    result.data |= r | l | u | d;

    return result;
};

// Returns the bitfield with only the 12 lower bits
FieldBit FieldBit::get_mask_12()
{
    FieldBit result = *this;
    result.data &= _mm_set_epi16(0, 0, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF);

    return result;
};

// Returns the bitfield with only the 13 lower bits
FieldBit FieldBit::get_mask_13()
{
    FieldBit result = *this;
    result.data &= _mm_set_epi16(0, 0, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF);

    return result;
};

// Returns the mask of all the poppable group
// A group is considered poppable if that group's count is bigger than 3
FieldBit FieldBit::get_mask_pop()
{
    // This is the same as BIT_AND mask 0xFFF all 6 columns
    // We only need the 12 lower bits of each column because only the puyos that are under the 13th row can be popped
    __m128i m12 = this->get_mask_12().data;

    // Shifts the whole mask once then BIT_AND with the original m12 mask
    // Do this for al 4 directions
    __m128i r = _mm_srli_si128(m12, 2) & m12; // shifts m12 right then BIT_AND m12 -> finds all bits that have a right connection
    __m128i l = _mm_slli_si128(m12, 2) & m12; // shifts m12 left then BIT_AND m12 -> finds all bits that have a left connection
    __m128i u = _mm_srli_epi16(m12, 1) & m12; // shifts m12 up then BIT_AND m12 -> finds all bits that have a up connection
    __m128i d = _mm_slli_epi16(m12, 1) & m12; // shifts m12 down then BIT_AND m12 -> finds all bits that have a down connection

    // Gets BIT_AND of shifted up mask and shifted down mask
    // This returns all the bits that are connected both up and down
    __m128i ud_and = u & d;

    // Gets BIT_AND of shifted right mask and shifted left mask
    // This returns all the bits that are connected both left and right
    __m128i lr_and = l & r;

    // Gets BIT_OR of shifted up mask and shifted down mask
    // This returns all the bits that have at least 1 vertical connection
    __m128i ud_or = u | d;

    // Gets BIT_OR of shifted right mask and shifted left mask
    // This returns all the bits that have at least 1 horizontal connection
    __m128i lr_or = l | r;

    // Finds all the bits that have at least 3 connections
    // Ex:
    // ......      ......      ......
    // ..X...  ->  ..X...  ->  ......
    // .XXX..  ->  .XOX..  ->  ..X...
    // ......      ......      ......
    __m128i m3 = (ud_and & lr_or) | (lr_and & ud_or);

    // Finds all the bits that have at least 2 connections
    // Ex:
    // ......      ......      ......
    // ...X..  ->  ...X..  ->  ......
    // .XXX..  ->  .XOO..  ->  ..XX..
    // ....X.      ....X.      ......
    // ...XX.      ...XO.      ....X.
    __m128i m2 = ud_and | lr_and | (ud_or & lr_or);

    // Shifts the m2 mask for all directions then BIT_ANDs with itself to find all the 2-connected bits that are connected to each other
    // Ex:
    // only right shift
    // ......      ......
    // ...X..  ->  ...... 
    // .XOO..  ->  ..X...
    // ....X.      ......
    // ...XO.      ......
    __m128i m2_r = _mm_srli_si128(m2, 2) & m2;
    __m128i m2_l = _mm_slli_si128(m2, 2) & m2;
    __m128i m2_u = _mm_srli_epi16(m2, 1) & m2;
    __m128i m2_d = _mm_slli_epi16(m2, 1) & m2;

    // This returns the mask of:
    // - bits that have at least 3 connections
    // - 2-connected bits that are connected to each other
    // We can then expand this mask for 4 directions to find the poppable mask
    FieldBit result = FieldBit();
    result.data = (m3 | m2_r | m2_l | m2_u | m2_d);

    // Shifts the mask for all direction and BIT_OR all of them with the original mask
    // This is the same as:
    // result = result | result_shifted_right | result_shifted_left | result_shifted_up | result_shifted_down;
    // Ex:
    // ......      ......
    // ......      .XX...
    // .XX...  ->  XXXX..
    // ......      .XX..X
    // .....X      ....XX
    result = result.get_expand();

    // Finally, BIT_ANDs with the original m12 mask
    // This is the mask of all bits that should be cleared
    result.data &= m12;

    return result;
};

// Flood-fills from the input bit and returns the filled mask
FieldBit FieldBit::get_mask_group(i8 x, i8 y)
{
    __m128i m12 = this->get_mask_12().data;

    FieldBit m = FieldBit();
    m.set_bit(x, y);

    while (true)
    {
        __m128i m_expand = m.get_expand().data & m12;

        if (_mm_testc_si128(m.data, m_expand)) {
            break;
        }

        m.data = m_expand;
    }

    return m;
};

// Expands 3 times from the input bit and returns the filled mask
FieldBit FieldBit::get_mask_group_4(i8 x, i8 y)
{
    __m128i m12 = this->get_mask_12().data;

    FieldBit m = FieldBit();
    m.set_bit(x, y);

    for (i32 i = 0; i < 3; ++i) {
        __m128i m_expand = m.get_expand().data & m12;

        if (_mm_testc_si128(m.data, m_expand)) {
            break;
        }

        m.data = m_expand;
    }

    return m;
};

// Flood-fills from the least significant bit and returns the filled mask
FieldBit FieldBit::get_mask_group_lsb()
{
    __m128i m12 = this->get_mask_12().data;

    alignas(16) u64 v[2];
    _mm_store_si128((__m128i*)v, m12);

    if (v[0] == 0) {
        v[1] &= (~v[1] + 1);
    }
    else {
        v[0] &= (~v[0] + 1);
        v[1] = 0;
    }

    FieldBit m;
    m.data = _mm_load_si128((const __m128i*)v);

    while (true)
    {
        __m128i m_expand = m.get_expand().data & m12;

        if (_mm_testc_si128(m.data, m_expand)) {
            break;
        }

        m.data = m_expand;
    }

    return m;
};

// Returns if the bitfield is empty
bool FieldBit::is_empty()
{
    return _mm_testz_si128(this->data, this->data);
};

// Pops the bitfield
void FieldBit::pop(FieldBit& mask)
{
    alignas(16) u16 v[8];
    alignas(16) u16 v_mask[8];

    _mm_store_si128((__m128i*)v, this->data);
    _mm_store_si128((__m128i*)v_mask, mask.data);

    for (i32 i = 0; i < 6; ++i) {
        v[i] = pext16(v[i], ~v_mask[i]);
    }

    this->data = _mm_load_si128((const __m128i*)v);
};

void FieldBit::print()
{
    for (int i = 5; i >= 0; --i) {
        std::cout << std::bitset<13>(this->get_col(i)) << "\n";
    }
    
    std::cout << std::endl;
};