#include "move.h"

namespace Move
{

avec<Placement, 22> generate(Field& field, bool pair_equal)
{
    avec<Placement, 22> result = avec<Placement, 22>();

    u8 heights[6];
    field.get_heights(heights);

    if (heights[2] > 11) {
        return result;
    }

    for (i8 x = 0; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, Direction::Type::UP)) {
            result.add({ x, Direction::Type::UP });
        }
    }

    for (i8 x = 0; x < 5; ++x) {
        if (is_valid(heights, field.row14, x, Direction::Type::RIGHT)) {
            result.add({ x, Direction::Type::RIGHT });
        }
    }

    if (pair_equal) {
        return result;
    }

    for (i8 x = 0; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, Direction::Type::DOWN)) {
            result.add({ x, Direction::Type::DOWN });
        }
    }

    for (i8 x = 1; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, Direction::Type::LEFT)) {
            result.add({ x, Direction::Type::LEFT });
        }
    }

    return result;
};

bool is_valid(u8 heights[6], u8 row14, i8 x, Direction::Type r)
{
    if (heights[x] + (r == Direction::Type::DOWN) > 12) {
        return false;
    }

    i8 child_x = x + Direction::get_offset_x(r);
    i8 child_y = heights[child_x] + (r == Direction::Type::UP);

    if (child_y == 13 && ((row14 >> child_x) & 1)) {
        return false;
    }

    const i8 check[6][4] = {
        { 1, 0, -1 },
        { 1, -1 },
        { -1 },
        { 3, -1 },
        { 3, 4, -1 },
        { 3, 4, 5, -1 },
    };

    const i8 check_12[6][6] = {
        { 1, 2, 3, 4, 5, -1 },
        { 2, 3, 4, 5, -1 },
        { -1 },
        { 2, 1, 0, -1 },
        { 3, 2, 1, 0, -1 },
        { 4, 3, 2, 1, 0, -1 },
    };

    i8 check_x = x;
    if (r == Direction::Type::RIGHT && x >= 2) {
        check_x += 1;
    }
    else if (r == Direction::Type::LEFT && x <= 2) {
        check_x -= 1;
    }

    i8 height_12_idx = -1;
    for (i8 i = 0; check[check_x][i] != -1; ++i) {
        if (heights[check[check_x][i]] > 12) {
            return false;
        }

        if (heights[check[check_x][i]] == 12 && height_12_idx == -1) {
            height_12_idx = check[check_x][i];
        }
    }

    if (height_12_idx == -1) {
        return true;
    }

    if (heights[1] > 11 && heights[3] > 11) {
        return true;
    }

    for (i8 i = 0; check_12[height_12_idx][i] != -1; ++i) {
        if (heights[check_12[height_12_idx][i]] > 11) {
            break;
        }

        if (heights[check_12[height_12_idx][i]] == 11) {
            return true;
        }
    }

    return false;
};

};