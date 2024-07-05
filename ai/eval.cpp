#include "eval.h"

namespace Eval
{

Result evaluate(Field& field, i32 tear, i32 waste, Weight& w)
{
    i32 eval = 0;

    auto plan = field;

    const i32 MAX_DEPTH = 6;
    const i32 MAX_DROP = 3;

    i32 q = INT32_MIN;
    i32 q_max = INT32_MIN;

    i32 dub_2 = 0;

    Quiet::search(field, MAX_DEPTH, MAX_DROP, [&] (Quiet::Result quiet) {
        i32 q_score = 0;

        q_max = std::max(q_max, quiet.score);

        if (quiet.chain == 2 && quiet.score >= 840) {
            dub_2 = std::max(dub_2, quiet.score);
        }

        u8 heights[6];
        quiet.plan.get_heights(heights);
        heights[quiet.x] = field.get_height(quiet.x);

        q_score += quiet.chain * w.chain;
        q_score += heights[quiet.x] * w.y;

        i32 need = quiet.plan.get_height(quiet.x) - heights[quiet.x];
        q_score += need * w.need;

        i32 key = quiet.plan.get_count() - field.get_count() - need;
        q_score += key * w.key;

        i32 chi = 0;
        if (quiet.x < 5) {
            for (i32 i = quiet.x + 1; i < 6; ++i) {
                if (heights[i] > heights[quiet.x]) {
                    break;
                }

                chi += 1;
            }
        }
        if (quiet.x > 0) {
            for (i32 i = quiet.x - 1; i >= 0; --i) {
                if (heights[i] > heights[quiet.x]) {
                    break;
                }

                chi += 1;
            }
        }
        q_score += chi * w.chi;

        i32 link_h = Eval::get_link_horizontal(quiet.remain);
        q_score += link_h * w.link_h;

        if (q_score > q) {
            q = q_score;
            plan = quiet.plan;
        }
    });

    if (q > INT32_MIN) {
        eval += q;
        eval += (dub_2 >> 8) * w.dub_2;
    }

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

        eval += form * w.form;
    }

    i32 shape = Eval::get_shape(heights);
    eval += shape * w.shape;

    i32 well = Eval::get_well(heights);
    eval += well * w.well;

    i32 bump = Eval::get_bump(heights);
    eval += bump * w.bump;

    i32 link = Eval::get_link(field);
    eval += link * w.link;

    i32 waste_14 = Eval::get_waste_14(field.row14);
    eval += waste_14 * w.waste_14;

    eval += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    i32 height_left = heights[0] + heights[1];
    i32 height_right = heights[3] + heights[4] + heights[5];
    eval += (std::max(height_left, height_right) - i32(heights[2])) * w.side;

    eval += tear * w.tear;

    eval += waste * w.waste;

    return Result {
        .value = eval,
        .plan = plan,
        .q = q_max
    };
};

i32 get_shape(u8 heights[6])
{
    i32 shape = 0;
    i32 shape_coef[6] = { 2, 0, -2, -2, 0, 2 };

    i32 height_avg = 0;

    for (i32 i = 0; i < 6; ++i) {
        height_avg += heights[i];
    }

    height_avg = height_avg / 6;

    for (i32 i = 0; i < 6; ++i) {
        shape += std::abs(i32(heights[i]) - height_avg - shape_coef[i]);
    }

    return shape;
};

i32 get_well(u8 heights[6])
{
    i32 well = 0;

    if (heights[0] < heights[1]) {
        well += heights[1] - heights[0];
    }

    if (heights[5] < heights[4]) {
        well += heights[4] - heights[5];
    }

    for (i32 i = 1; i < 5; ++i) {
        if (heights[i] < heights[i - 1] && heights[i] < heights[i + 1]) {
            well += std::min(heights[i - 1], heights[i + 1]) - heights[i];
        }
    }

    return well;
};

i32 get_bump(u8 heights[6])
{
    i32 bump = 0;

    for (i32 i = 1; i < 5; ++i) {
        if (heights[i] > heights[i - 1] && heights[i] > heights[i + 1]) {
            bump += heights[i] - std::max(heights[i - 1], heights[i + 1]);
        }
    }

    return bump;
};

i32 get_link(Field& field)
{
    i32 link = 0;

    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        __m128i m12 = field.data[p].get_mask_12().data;

        FieldBit hor;
        hor.data = _mm_slli_si128(m12, 2) & m12;
        link += hor.get_count();

        FieldBit ver;
        ver.data = _mm_slli_epi16(m12, 1) & m12;
        link += ver.get_count();
    }

    return link;
};

i32 get_link_horizontal(Field& field)
{
    i32 link = 0;

    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        __m128i m12 = field.data[p].get_mask_12().data;

        FieldBit hor;
        hor.data = _mm_slli_si128(m12, 2) & m12;
        link += hor.get_count();
    }

    return link;
};

i32 get_waste_14(u8 row14)
{
    i32 space = 1;

    for (i32 i = 3; i < 6; ++i) {
        if ((row14 >> i) & 1) {
            break;
        }

        space += 1;
    }

    for (i32 i = 1; i >= 0; --i) {
        if ((row14 >> i) & 1) {
            break;
        }

        space += 1;
    }

    return 6 - space;
};

};