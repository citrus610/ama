#include "eval.h"

namespace dfs
{

namespace eval
{


// Evalutes a field
Result evaluate(Field& field, i32 tear, i32 waste, const Weight& w)
{
    i32 eval = 0;

    i32 count = field.get_count();

    // Quiescence search
    // Searching past the 2 given puyo pairs by dropping puyos until there aren't any chains left
    // This estimates the potential of the field
    auto plan = field;

    i32 q = INT32_MIN;
    i32 q_max = INT32_MIN;

    quiet::search(field, 16, 3, [&] (quiet::Result quiet) {
        i32 q_score = 0;

        u8 heights[6];
        quiet.plan.get_heights(heights);
        heights[quiet.x] = field.get_height(quiet.x);

        // Potential chain
        q_score += quiet.chain.count * w.chain;

        // Trigger height
        q_score += i32(heights[quiet.x]) * w.y;

        // Key puyos needed
        i32 key = quiet.plan.get_count() - count;
        q_score += key * w.key;

        // Space for stretching chain
        i32 chi = eval::get_chi(heights, quiet.x);
        q_score += chi * w.chi;

        // Remaining connections
        auto [link_2, link_3] = eval::get_link_23(quiet.remain);
        q_score += link_2 * w.link_2;
        q_score += link_3 * w.link_3;

        // Updates the best q score and plan
        if (q_score > q) {
            q = q_score;
            q_max = quiet.chain.score;
            plan = quiet.plan;
        }
    });

    if (q > INT32_MIN) {
        eval += q;
    }

    // Static evaluation value
    eval += eval::get_static(field, w);

    u8 heights[6];
    field.get_heights(heights);

    // Avoids wasting space on the 14th row
    i32 waste_14 = eval::get_waste_14(field.row14);
    eval += waste_14 * w.waste_14;

    // Avoids garbage puyo
    eval += field.data[static_cast<u8>(cell::Type::GARBAGE)].get_count() * w.nuisance;

    // Field side bias
    i32 height_left = heights[0] + heights[1];
    i32 height_right = heights[3] + heights[4] + heights[5];
    eval += (std::max(height_left, height_right) - i32(heights[2])) * w.side;

    // Avoids tearing
    eval += tear * w.tear;

    // Avoids wasting resource by popping puyos
    eval += waste * w.waste;

    return Result {
        .value = eval,
        .q = q_max,
        .plan = plan
    };
};

// Returns static eval
i32 get_static(Field& field, const Weight& w)
{
    i32 eval = 0;

    u8 heights[6];
    field.get_heights(heights);

    // Field's shape
    const i32 shape_coef[6] = { 2, 2, 2, -2, -2, -2 };
    eval += eval::get_shape(heights, shape_coef) * w.shape;

    // Field's u shape
    i32 u = eval::get_u(heights);
    eval += u * w.u;

    // Avoids wells
    i32 well = eval::get_well(heights);
    eval += well * w.well;

    // Avoids bumps
    i32 bump = eval::get_bump(heights);
    eval += bump * w.bump;

    // Puyo connections
    auto [link_2, link_3] = eval::get_link_23(field);
    eval += link_2 * w.link_2;
    eval += link_3 * w.link_3;

    return eval;
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
i32 get_shape(u8 heights[6], const i32 coef[6])
{
    i32 shape = 0;

    i32 height_avg = 0;

    for (i32 i = 0; i < 6; ++i) {
        height_avg += heights[i];
    }

    height_avg = height_avg / 6;

    for (i32 i = 0; i < 6; ++i) {
        shape += std::abs(i32(heights[i]) - height_avg - coef[i]);
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

// Evaluates the field's u shape
// A field's shape is considered to be an u shape when the columns in the middle are lower than the columns on the outside
i32 get_u(u8 heights[6])
{
    i32 u = 0;

    u += std::max(0, i32(heights[2]) - i32(heights[1]));
    u += std::max(0, i32(heights[2]) - i32(heights[0]));
    u += std::max(0, i32(heights[1]) - i32(heights[0]));

    // u += std::max(0, i32(heights[3]) - i32(heights[4]));
    // u += std::max(0, i32(heights[3]) - i32(heights[5]));
    // u += std::max(0, i32(heights[4]) - i32(heights[5]));

    return u;
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