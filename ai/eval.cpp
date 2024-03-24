#include "eval.h"

namespace Eval
{

Result evaluate(Field& field, i32 tear, i32 waste, Weight& w, i32 depth, i32 drop)
{
    i32 result = 0;

    auto plan = field;

    i32 field_count = field.get_count();

    const i32 phase_min = 36;
    const i32 phase_max = 48;
    const i32 phase_dt = phase_max - phase_min;

    i32 phase = std::clamp(field_count, phase_min, phase_max) - phase_min;

    i32 dub_2 = 0;
    i32 dub_3 = 0;

    i32 q_max = 0;

    i32 qscore = INT32_MIN;
    Quiet::search(field, depth, drop, [&] (Quiet::Result q) {
        i32 score = 0;

        if (q.chain == 2 && q.score >= 840) {
            dub_2 += 1;
        }

        if (q.chain == 3 && q.score >= 1680) {
            dub_3 += 1;
        }

        u8 heights[6];
        q.plan.get_heights(heights);
        heights[q.x] = field.get_height(q.x);

        i32 q_plan_count = q.plan.get_count();

        q_max = std::max(
            q_max,
            q.score * (78 - q_plan_count) / (78 - field_count)
        );

        score += q.chain * w.chain;
        score += q.score * w.score;
        score += heights[q.x] * w.y;

        i32 x_phase_min = std::abs(q.x - 0) * w.x;
        i32 x_phase_max = std::abs(q.x - 5) * w.x;
        score += (x_phase_min * (phase_dt - phase) + x_phase_max * phase) / phase_dt;

        i32 need = q.plan.get_height(q.x) - heights[q.x];
        score += need * w.need;

        i32 key = q.plan.get_count() - field_count - need;
        score += key * w.key;

        i32 chi = 0;
        if (q.x < 5) {
            for (i32 i = q.x + 1; i < 6; ++i) {
                if (heights[i] > heights[q.x]) {
                    break;
                }

                chi += 1;
            }
        }
        if (q.x > 0) {
            for (i32 i = q.x - 1; i >= 0; --i) {
                if (heights[i] > heights[q.x]) {
                    break;
                }

                chi += 1;
            }
        }
        score += (chi - 5) * w.chi;

        if (score > qscore) {
            qscore = score;
            plan = q.plan;
        }
    });
    if (qscore > INT32_MIN) {
        result += qscore;
    }

    result += dub_2 * w.dub_2;
    result += dub_3 * w.dub_3;

    u8 heights[6];
    field.get_heights(heights);

    if (w.form > 0) {
        i32 form = -10;

        const Form::Data list[] = {
            Form::GTR_2(),
            Form::GTR_1(),
            Form::FRON(),
            Form::MERI(),
            Form::SGTR(),
            Form::LLR_1(),
            Form::LLR_2()
        };

        auto mask_garbage = field.data[static_cast<i32>(Cell::Type::GARBAGE)];
        mask_garbage.data &= _mm_set_epi16(0, 0, 0, 0, 0xF, 0xF, 0xF, 0xF);

        if (mask_garbage.get_count() > 0) {
            form = 0;
        }
        else {
            for (i32 i = 0; i < _countof(list); ++i) {
                form = std::max(form, Form::evaluate(field, heights, list[i]));
            }
        }

        result += form * w.form;
    }

    i32 shape_phase_min = 0;
    i32 shape_phase_max = 0;
    Eval::get_shape(heights, shape_phase_min, shape_phase_max);
    result += (shape_phase_min * (phase_dt - phase) + shape_phase_max * phase) * w.shape / phase_dt;
    // result += shape_phase_min * w.shape;

    i32 u = Eval::get_u(heights);
    result += u * w.u;

    i32 space14 = 1;
    for (i32 i = 3; i < 6; ++i) {
        if ((field.row14 >> i) & 1) {
            break;
        }

        space14 += 1;
    }
    for (i32 i = 1; i >= 0; --i) {
        if ((field.row14 >> i) & 1) {
            break;
        }

        space14 += 1;
    }
    result += space14 * w.space14;

    i32 link = Eval::get_link(field);
    result += link * w.link;

    i32 link_h = Eval::get_link_horizontal(field);
    result += link_h * w.link_h;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += (i32(heights[3]) + i32(heights[4]) + i32(heights[5]) - i32(heights[0]) - i32(heights[1]) - i32(heights[2])) * w.side;

    result += tear * w.tear;

    result += waste * w.waste;

    return Result {
        .value = result,
        .plan = plan,
        .q = q_max
    };
};

void get_shape(u8 heights[6], i32& shape_phase_min, i32& shape_phase_max)
{
    i32 shape_coef_min[6] = { 2, 0, 0, -4, -4, -2 };
    i32 shape_coef_max[6] = { 2, 0, 0, 0, 0, 2 };

    for (i32 i = 0; i < 6; ++i) {
        shape_phase_min += std::abs(i32(heights[i]) - i32(heights[2]) - shape_coef_min[i]);
        shape_phase_max += std::abs(i32(heights[i]) - i32(heights[2]) - shape_coef_max[i]);
    }
};

i32 get_u(u8 heights[6])
{
    i32 u = 0;

    u += std::max(0, i32(heights[1]) - i32(heights[0]));
    u += std::max(0, i32(heights[2]) - i32(heights[1]));
    u += std::max(0, i32(heights[4]) - i32(heights[5]));
    u += std::max(0, i32(heights[3]) - i32(heights[4]));

    return u;
};

i32 get_link(Field& field)
{
    i32 result = 0;

    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        __m128i m12 = field.data[p].get_mask_12().data;

        FieldBit lh;
        FieldBit lv;

        lh.data = _mm_srli_si128(m12, 2) & m12;
        lv.data = _mm_srli_epi16(m12, 1) & m12;

        result += lh.get_count() + lv.get_count();
    }

    return result;
};

i32 get_link_horizontal(Field& field)
{
    i32 result = 0;

    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        __m128i m12 = field.data[p].get_mask_12().data;

        alignas(16) u16 v[8];
        _mm_store_si128((__m128i*)v, m12);

        result += std::popcount(u32(v[0] & v[1]));
        result += std::popcount(u32(v[1] & v[2]));
        result += std::popcount(u32(v[3] & v[4]));
        result += std::popcount(u32(v[4] & v[5]));
    }

    return result;
};

};