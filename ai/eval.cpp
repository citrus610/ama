#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 tear, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    if (detect.has_value()) {
        result += (detect->score.chain.score >> 8) * w.chain_score;
        result += detect->score.chain.count * w.chain_count;
        result += detect->score.height * w.chain_height;
        result += detect->score.needed * w.chain_needed;

        const i32 x_coef[6] = { 0, 2, 1, -1, -2, -3};
        result += x_coef[detect->score.x] * w.chain_x;
    }

    i32 u = 0;
    // u += std::max(0, heights[1] - heights[0]);
    // // u += std::max(0, heights[2] - heights[1]);
    // // u += std::max(0, heights[3] - heights[4]);
    // u += std::max(0, heights[4] - heights[5]);
    // result += u * w.u;

    // i32 bump_mid = 0;
    // i32 highest_mid = *std::max_element(heights + 1, heights + 5);
    // for (i32 i = 1; i < 5; ++i) {
    //     bump_mid += highest_mid - heights[i];
    // }
    // result += bump_mid * w.bump_mid;
    const i32 coef[6] = { 2, 0, 0, 0, 0, 2 };
    // const i32 coef_mid[6] = { 3, 1, 0, 0, 1, 3 };
    // auto coef = (field.get_count() > 24) ? coef_mid : coef_open;
    auto max_height_mid = std::max(heights[2], heights[3]);
    for (i32 i = 0; i < 6; ++i) {
        u += std::abs(heights[i] - max_height_mid - coef[i]);
    }
    result += u * w.u;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += std::max(0, heights[0] + heights[1] + heights[2] - heights[3] - heights[4] - heights[5]) * w.side_bias;

    i32 link_hor_2 = 0;
    i32 link_hor_3 = 0;
    i32 link_ver_2 = 0;
    i32 link_ver_3 = 0;
    i32 link_mid = 0;
    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        FieldBit hor;
        hor.data = _mm_slli_si128(m12.data, 2) & m12.data;
        i32 hor_2 = hor.get_count();
        hor.data = _mm_slli_si128(hor.data, 2) & hor.data;
        i32 hor_3 = hor.get_count();
        hor_2 -= hor_3;
        link_hor_2 += hor_2;
        link_hor_3 += hor_3;

        FieldBit ver;
        ver.data = _mm_slli_epi16(m12.data, 1) & m12.data;
        i32 ver_2 = ver.get_count();
        ver.data = _mm_slli_epi16(ver.data, 1) & ver.data;
        i32 ver_3 = ver.get_count();
        ver_2 -= ver_3;
        link_ver_2 += ver_2;
        link_ver_3 += ver_3;

        hor.data = m12.data & _mm_set_epi16(0, 0, 0, 0, 0xffff, 0xffff, 0, 0);
        hor.data = _mm_slli_si128(hor.data, 2) & hor.data;
        link_mid += hor.get_count();
    }
    result += link_hor_2 * w.link_hor_2;
    result += link_hor_3 * w.link_hor_3;
    result += link_ver_2 * w.link_ver_2;
    result += link_ver_3 * w.link_ver_3;
    result += link_mid * w.link_mid;

    if (w.form != 0) {
        result += Form::evaluate(field, heights, Form::DEFAULT()) * w.form;
    }

    result += tear * w.tear;

    return result;
};

};