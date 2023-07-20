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

void FieldBit::set_bit(i8 x, i8 y)
{
    assert(x >= 0 && x < 6);
    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);
    v[x] |= 1 << y;
    this->data = _mm_load_si128((const __m128i*)v);
};

bool FieldBit::get_bit(i8 x, i8 y)
{
    if (x < 0 || x > 5 || y < 0 || y > 12) return true;
    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);
    return v[x] & (1 << y);
};

u32 FieldBit::get_count()
{
    alignas(16) u64 v[2];
    _mm_store_si128((__m128i*)v, this->data);
    return std::popcount(v[0]) + std::popcount(v[1]);
};

u16 FieldBit::get_col(i8 x)
{
    assert(x >= 0 && x < 6);
    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, this->data);
    return v[x];
};

FieldBit FieldBit::get_expand()
{
    __m128i r = _mm_srli_si128(this->data, 2);
    __m128i l = _mm_slli_si128(this->data, 2);
    __m128i u = _mm_srli_epi16(this->data, 1);
    __m128i d = _mm_slli_epi16(this->data, 1);
    FieldBit result = *this;
    result.data |= r | l | u | d;
    return result;
};

FieldBit FieldBit::get_mask_12()
{
    FieldBit result = *this;
    result.data &= _mm_set_epi16(0, 0, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF);
    return result;
};

FieldBit FieldBit::get_mask_13()
{
    FieldBit result = *this;
    result.data &= _mm_set_epi16(0, 0, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF);
    return result;
};

FieldBit FieldBit::get_mask_pop()
{
    __m128i m12 = this->get_mask_12().data;

    __m128i r = _mm_srli_si128(m12, 2) & m12;
    __m128i l = _mm_slli_si128(m12, 2) & m12;
    __m128i u = _mm_srli_epi16(m12, 1) & m12;
    __m128i d = _mm_slli_epi16(m12, 1) & m12;

    __m128i ud_and = u & d;
    __m128i lr_and = l & r;
    __m128i ud_or = u | d;
    __m128i lr_or = l | r;

    __m128i m3 = (ud_and & lr_or) | (lr_and & ud_or);
    __m128i m2 = ud_and | lr_and | (ud_or & lr_or);
    __m128i m2_r = _mm_srli_si128(m2, 2) & m2;
    __m128i m2_l = _mm_slli_si128(m2, 2) & m2;
    __m128i m2_u = _mm_srli_epi16(m2, 1) & m2;
    __m128i m2_d = _mm_slli_epi16(m2, 1) & m2;

    FieldBit result = FieldBit();
    result.data = (m3 | m2_r | m2_l | m2_u | m2_d);
    result = result.get_expand();
    result.data &= m12;
    return result;
};

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

FieldBit FieldBit::get_mask_group_4(i8 x, i8 y)
{
    __m128i m12 = this->get_mask_12().data;

    FieldBit m = FieldBit();
    m.set_bit(x, y);

    for (i32 i = 0; i < 4; ++i) {
        __m128i m_expand = m.get_expand().data & m12;
        if (_mm_testc_si128(m.data, m_expand)) {
            break;
        }
        m.data = m_expand;
    }

    return m;
};

FieldBit FieldBit::get_mask_group_lsb()
{
    __m128i m12 = this->get_mask_12().data;

    alignas(16) u64 v[2];
    _mm_store_si128((__m128i*)v, m12);
    // if (v[0] == 0 && v[1] == 0) return FieldBit();

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

bool FieldBit::is_empty()
{
    return _mm_testz_si128(this->data, this->data);
};

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