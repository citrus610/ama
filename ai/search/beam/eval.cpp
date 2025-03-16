#include "eval.h"

namespace beam
{

namespace eval
{

void evaluate(node::Data& node, i32 tear, i32 waste, const Weight& w)
{
    node.score.eval = 0;

    u8 heights[6];
    node.field.get_heights(heights);

    // Human form pattern matching
    if (w.form > 0) {
        i32 form = -100;

        const form::Data list[] = {
            form::GTR(),
            form::SGTR(),
            form::FRON()
        };

        // Stop pattern matching if we have garbage puyo
        auto mask_garbage = node.field.data[static_cast<i32>(cell::Type::GARBAGE)];
        mask_garbage.data &= _mm_set_epi16(0, 0, 0, 0, 0xF, 0xF, 0xF, 0xF);

        if (mask_garbage.get_count() > 0) {
            form = 0;
        }
        else {
            // Find the best matching form
            for (i32 i = 0; i < _countof(list); ++i) {
                form = std::max(form, form::evaluate(node.field, heights, list[i]));
            }
        }

        node.score.eval += form * w.form;
    }

    // Quiescence search
    i32 q = INT32_MIN;

    quiet::search(node.field, 3, [&] (quiet::Result quiet) {
        i32 q_score = 0;

        // Potential chain
        q_score += quiet.chain.count * w.chain;

        // Trigger height
        q_score += heights[quiet.x] * w.y;

        // Key puyos needed
        q_score += quiet.key * w.key;

        // Space for stretching chain
        i32 chi = eval::get_chi(heights, quiet.x);
        q_score += chi * w.chi;

        // Remaining connection
        auto [link_2, link_3] = eval::get_link_23(quiet.remain);
        q_score += link_2 * w.link_2;
        q_score += link_3 * w.link_3;

        // Updates the best q score and plan
        q = std::max(q, q_score);
    });

    if (q > INT32_MIN) {
        node.score.eval += q;
    }

    // Field's shape
    i32 shape = eval::get_shape(heights);
    node.score.eval += shape * w.shape;

    // Avoids wells
    i32 well = eval::get_well(heights);
    node.score.eval += well * w.well;

    // Avoids bumps
    i32 bump = eval::get_bump(heights);
    node.score.eval += bump * w.bump;

    // Puyo connections
    auto [link_2, link_3] = eval::get_link_23(node.field);
    node.score.eval += link_2 * w.link_2;
    node.score.eval += link_3 * w.link_3;

    // Avoids wasting space on the 14th row
    i32 waste_14 = eval::get_waste_14(node.field.row14);
    node.score.eval += waste_14 * w.waste_14;

    // Avoids garbage puyo
    node.score.eval += node.field.data[static_cast<u8>(cell::Type::GARBAGE)].get_count() * w.nuisance;

    // Field side bias
    i32 height_left = heights[0] + heights[1];
    i32 height_right = heights[3] + heights[4] + heights[5];
    node.score.eval += (std::max(height_left, height_right) - i32(heights[2])) * w.side;

    // Avoids tearing
    node.score.action += tear * w.tear;

    // Avoids wasting resource by popping puyos
    node.score.action += waste * w.waste;
};

// Returns how extendable the trigger point is
i32 get_chi(u8 heights[6], i8 x)
{
    i32 chi = 0;

    if (x < 5) {
        for (auto i = x + 1; i < 6; ++i) {
            if (heights[i] > heights[x]) {
                break;
            }

            chi += 1;
        }

        for (auto i = x + 1; i < 6; ++i) {
            if (heights[i] >= heights[x]) {
                break;
            }

            chi += 1;
        }
    }

    if (x > 0) {
        for (auto i = x - 1; i >= 0; --i) {
            if (heights[i] > heights[x]) {
                break;
            }

            chi += 1;
        }

        for (auto i = x - 1; i >= 0; --i) {
            if (heights[i] >= heights[x]) {
                break;
            }

            chi += 1;
        }
    }

    return chi;
};

// Returns how close the field's shape is to the ideal shape
i32 get_shape(u8 heights[6])
{
    i32 shape = 0;

    // We define the ideal field shape here:
    // ......
    // ......
    // ......
    // ###...
    // ###...
    // ######
    // ######
    // ######
    const i32 shape_coef[6] = { 1, 1, 1, -1, -1, -1 };

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

// Returns the field's well depth
// If a column is lower than its 2 nearby columns, we consider that a well
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

// Evaluates the field's bumpiness
// If a column is higher than its 2 nearby columns, it's considered a bump
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

// Returns the number connections in the field
i32 get_link(Field& field)
{
    i32 link = 0;

    for (u8 p = 0; p < cell::COUNT - 1; ++p) {
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

// Returns the number of 2-connected and 3-connected links in the field
std::pair<i32, i32> get_link_23(Field& field)
{
    i32 link_2 = 0;
    i32 link_3 = 0;

    for (u8 p = 0; p < cell::COUNT - 1; ++p) {
        __m128i m12 = field.data[p].get_mask_12().data;

        __m128i r = _mm_srli_si128(m12, 2) & m12;
        __m128i l = _mm_slli_si128(m12, 2) & m12;
        __m128i u = _mm_srli_epi16(m12, 1) & m12;
        __m128i d = _mm_slli_epi16(m12, 1) & m12;

        __m128i ud_and = u & d;
        __m128i lr_and = l & r;
        __m128i ud_or = u | d;
        __m128i lr_or = l | r;

        FieldBit l3;
        FieldBit l2;

        l3.data = (ud_or & lr_or) | ud_and | lr_and;
        l2.data = _mm_andnot_si128(l3.get_expand().data, u | l);

        link_2 += l2.get_count();
        link_3 += l3.get_count();
    }

    return { link_2, link_3 };
};

// Returns the remaining reachable cells left on the 14th row
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

};