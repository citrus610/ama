#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, i32 tear, i32 waste, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    i32 field_count = field.get_count();

    i32 detect = 0;
    Detect::detect(field, [&] (Detect::Result detect_result) {
        i32 score = 0;

        score += (detect_result.score.chain.score >> 8) * w.chain_score;
        score += detect_result.score.chain.count * w.chain_count;
        score += detect_result.score.height * w.chain_height;
        score += detect_result.score.needed * w.chain_needed;
        score += w.chain_x[detect_result.score.x];

        detect = std::max(detect, score);
    }, 3, 1);
    result += detect;

    i32 shape = 0;
    auto shape_coef = w.shape_coef_lo;
    if (field_count >= 30) {
        shape_coef = w.shape_coef_hi;
    }
    for (i32 i = 0; i < 6; ++i) {
        shape += std::abs(i32(heights[i]) - i32(heights[2]) - shape_coef[i]);
    }
    result += shape * w.shape;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += std::max(0, heights[0] + heights[1] + heights[2] - heights[3] - heights[4] - heights[5]) * w.side_bias;

    i32 link_hor_2 = 0;
    i32 link_hor_3 = 0;
    i32 link_ver_2 = 0;
    i32 link_ver_3 = 0;
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
    }
    result += link_hor_2 * w.link_hor_2;
    result += link_hor_3 * w.link_hor_3;
    result += link_ver_2 * w.link_ver_2;
    result += link_ver_3 * w.link_ver_3;

    if (w.form != 0) {
        i32 gtr = Form::evaluate(field, heights, Form::DEFAULT());

        // i32 gtr = -10;
        // for (i32 i = 0; i < 2; ++i) {
        //     gtr = Form::evaluate(field, heights, Form::GTR[i]);

        //     if (gtr >= 0) {
        //         break;
        //     }
        // }

        // if (gtr >= 0) {
        //     result += gtr * w.form;
        // }

        result += gtr * w.form;
    }

    result += tear * w.tear;

    result += waste * w.waste;

    return result;
};

};