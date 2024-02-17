#include "eval.h"

namespace Eval
{

Result evaluate(Field& field, i32 tear, i32 waste, Weight& w)
{
    i32 result = 0;

    auto plan = field;

    const i32 MAX_DEPTH = 6;
    const i32 MAX_DROP = 3;

    i32 dub_2 = 0;
    i32 dub_3 = 0;

    i32 qscore = INT32_MIN;
    Quiet::search(field, MAX_DEPTH, MAX_DROP, [&] (Quiet::Result q) {
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

        score += q.chain * w.chain;
        score += q.score * w.score;
        score += heights[q.x] * w.y;

        i32 need = q.plan.get_height(q.x) - heights[q.x];
        score += need * w.need;

        i32 key = MAX_DEPTH - q.depth;
        score += key * w.key;

        i32 key_s = q.plan.get_count() - field.get_count() - need - key;
        score += key_s * w.key_s;

        i32 chi = 0;
        if (q.x < 5) {
            for (i32 i = q.x + 1; i < 6; ++i) {
                if (heights[i] > heights[q.x]) {
                    break;
                }

                chi += 1;
            }
        }
        else if (q.x > 0) {
            for (i32 i = q.x - 1; i >= 0; --i) {
                if (heights[i] > heights[q.x]) {
                    break;
                }

                chi += 1;
            }
        }
        score += (chi - 6) * w.chi;

        i32 shape = Eval::get_shape(heights);
        score += shape * w.shape;

        i32 u = Eval::get_u(heights);
        score += u * w.u;

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

    if (qscore == INT32_MIN) {
        i32 shape = Eval::get_shape(heights);
        result += shape * w.shape;

        i32 u = Eval::get_u(heights);
        result += u * w.u;
    }

    i32 link_no = 0;
    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        FieldBit ln;
        ln.data = 
            (~_mm_slli_si128(m12.data, 2)) &
            (~_mm_srli_si128(m12.data, 2)) &
            (~_mm_slli_epi16(m12.data, 1)) &
            (~_mm_srli_epi16(m12.data, 1)) &
            m12.data;
        link_no += ln.get_count();
    }
    result += link_no * w.link_no;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += (i32(heights[3]) + i32(heights[4]) + i32(heights[5]) - i32(heights[0]) - i32(heights[1]) - i32(heights[2])) * w.side;

    result += tear * w.tear;

    result += waste * w.waste;

    return Result {
        .value = result,
        .plan = plan
    };
};

i32 get_shape(u8 heights[6])
{
    i32 shape = 0;
    i32 shape_coef[6] = { 2, 0, 0, -2, -2, 0 };

    for (i32 i = 0; i < 6; ++i) {
        shape += std::abs(i32(heights[i]) - i32(heights[2]) - shape_coef[i]);
    }

    return shape;
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

};