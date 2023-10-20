#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, i32 tear, i32 waste, i32 depth, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    if (w.form > 0) {
        i32 form = INT32_MIN;

        form = std::max(form, Form::evaluate(field, heights, Form::GTR_1()));
        form = std::max(form, Form::evaluate(field, heights, Form::GTR_2()));
        form = std::max(form, Form::evaluate(field, heights, Form::SGTR()));
        form = std::max(form, Form::evaluate(field, heights, Form::FRON()));
        form = std::max(form, Form::evaluate(field, heights, Form::MERI()));
        form = std::max(form, Form::evaluate(field, heights, Form::LLR_1()));
        form = std::max(form, Form::evaluate(field, heights, Form::LLR_2()));
        // form = std::max(form, Form::evaluate(field, heights, Form::YAYOI()));
        // form = std::max(form, Form::evaluate(field, heights, Form::SUBM()));

        result += form * w.form;
    }

    i32 field_count = field.get_count();
    i32 height_avrg = field_count / 6;

    i32 well[6] = { 0 };
    well[0] = (heights[1] > heights[0]) ? (heights[1] - heights[0]) : 0;
    well[5] = (heights[4] > heights[5]) ? (heights[4] - heights[5]) : 0;
    for (i32 i = 1; i < 5; ++i) {
        if (heights[i - 1] > heights[i] && heights[i + 1] > heights[i]) {
            well[i] = std::min(heights[i - 1], heights[i + 1]) - heights[i];
        }
    }

    i32 detect = 0;
    Qsearch::search(field, depth, 3, [&] (Qsearch::Result plan) {
        i32 score = 0;

        score += plan.chain * w.chain;
        score += (plan.score >> 8) * w.score;
        score += heights[plan.x] * w.y;
        score += (plan.plan.get_count() - field_count) * w.need;
        score += well[plan.x] * w.well;

        if (well[plan.x] == 0) {
            i32 extendable = 0;

            u8 plan_heights[6];
            plan.plan.get_heights(plan_heights);

            for (i32 i = plan.x; i < 5; ++i) {
                extendable += heights[plan.x] >= plan_heights[plan.x + 1];
            }

            for (i32 i = plan.x; i > 0; --i) {
                extendable += heights[plan.x] >= plan_heights[plan.x - 1];
            }

            score += extendable * w.extendable;
        }

        detect = std::max(detect, score);
    });
    result += detect;

    i32 shape = 0;
    std::array<i32, 6> shape_coef = { 2, 0, 0, 0, 0, 2 };
    if (field_count >= 30 && field_count < 48) {
        // shape_coef = { 3, 1, 0, 0, 1, 3 };
        shape_coef = { 2, 0, 0, -2, -2, 0 };
    }
    i32 height_mid = heights[2];
    for (i32 i = 0; i < 6; ++i) {
        shape += std::abs(i32(heights[i]) - height_mid - shape_coef[i]);
    }
    result += shape * w.shape;

    i32 u = 0;
    u += std::max(0, i32(heights[1]) - i32(heights[0]));
    u += std::max(0, i32(heights[2]) - i32(heights[1]));
    u += std::max(0, i32(heights[4]) - i32(heights[5]));
    u += std::max(0, i32(heights[3]) - i32(heights[4]));
    result += u * w.u;

    i32 link_h = 0;
    i32 link_v = 0;
    i32 link_no = 0;
    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        alignas(16) u16 v[8];
        _mm_store_si128((__m128i*)v, m12.data);
        link_h += std::popcount(u32(v[0] & v[1]));
        link_h += std::popcount(u32(v[1] & v[2]));
        link_h += std::popcount(u32(v[3] & v[4]));
        link_h += std::popcount(u32(v[4] & v[5]));

        FieldBit lv;
        link_v += std::popcount(u32(v[0] << 1) & u32(v[0]));
        link_v += std::popcount(u32(v[5] << 1) & u32(v[5]));

        FieldBit ln;
        ln.data = 
            (~_mm_slli_si128(m12.data, 2)) &
            (~_mm_srli_si128(m12.data, 2)) &
            (~_mm_slli_epi16(m12.data, 1)) &
            (~_mm_srli_epi16(m12.data, 1)) &
            m12.data;
        link_no += ln.get_count();
    }
    result += link_h * w.link_h;
    result += link_v * w.link_v;
    result += link_no * w.link_no;

    i32 link_top = 0;
    FieldBit visit = FieldBit();
    for (i32 x = 0; x < 6; ++x) {
        if (heights[x] == 0) {
            continue;
        }

        i32 check_max = 0;

        if (x == 0) {
            check_max = i32(heights[0]) - i32(heights[1]);
        }
        else if (x == 5) {
            check_max = i32(heights[5]) - i32(heights[4]);
        }
        else {
            check_max = i32(heights[x]) - i32(std::min(heights[x - 1], heights[x + 1]));
        }

        check_max = std::max(1, check_max);

        for (i32 k = 0; k < check_max; ++k) {
            i32 y = i32(heights[x]) - 1 - k;

            if (visit.get_bit(x, y)) {
                continue;
            }

            i32 p = static_cast<i32>(field.get_cell(x, y));

            auto mask_link = field.data[p].get_mask_group_4(x, y);

            visit = visit | mask_link;

            if (p == static_cast<i32>(Cell::Type::GARBAGE)) {
                continue;
            }

            FieldBit lh;
            lh.data = _mm_slli_si128(mask_link.data, 2) & mask_link.data;

            FieldBit lv;
            lv.data = _mm_slli_epi16(mask_link.data, 1) & mask_link.data;

            link_top += lh.get_count() + lv.get_count();
        }
    }
    result += link_top * w.link_top;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += (i32(heights[3]) + i32(heights[4]) + i32(heights[5]) - i32(heights[0]) - i32(heights[1]) - i32(heights[2])) * w.side;

    result += tear * w.tear;

    result += waste * w.waste;

    return result;
};

};