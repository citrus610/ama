#include "move.h"

namespace move
{

// Generates all reachable positions
avec<Placement, 22> generate(Field& field, bool pair_equal)
{
    avec<Placement, 22> result = avec<Placement, 22>();

    u8 heights[6];
    field.get_heights(heights);

    if (heights[2] > 11) {
        return result;
    }

    for (i8 x = 0; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, direction::Type::UP)) {
            result.add({ x, direction::Type::UP });
        }
    }

    for (i8 x = 0; x < 5; ++x) {
        if (is_valid(heights, field.row14, x, direction::Type::RIGHT)) {
            result.add({ x, direction::Type::RIGHT });
        }
    }

    if (pair_equal) {
        return result;
    }

    for (i8 x = 0; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, direction::Type::DOWN)) {
            result.add({ x, direction::Type::DOWN });
        }
    }

    for (i8 x = 1; x < 6; ++x) {
        if (is_valid(heights, field.row14, x, direction::Type::LEFT)) {
            result.add({ x, direction::Type::LEFT });
        }
    }

    return result;
};

// Checks if a position is reachable
bool is_valid(u8 heights[6], u8 row14, i8 x, direction::Type r)
{
    // Returns false if the axis puyo is placed on the 14th row
    if (heights[x] + (r == direction::Type::DOWN) > 12) {
        return false;
    }

    // Checks if the child puyo can be placed on the 14th row
    i8 child_x = x + direction::get_offset_x(r);
    i8 child_y = heights[child_x] + (r == direction::Type::UP);

    if (child_y == 13 && ((row14 >> child_x) & 1)) {
        return false;
    }

    // Check constants
    // Basically it's just the list of all positions that we have to check in order to reach the desire position
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

    // Finds the farest column that we have to check
    i8 check_x = x;

    if (r == direction::Type::RIGHT && x >= 2) {
        check_x += 1;
    }
    else if (r == direction::Type::LEFT && x <= 2) {
        check_x -= 1;
    }

    // Checks if we can drop puyos at the desire column
    // While finding out if we have to do a floor kick to reach there
    i8 height_12_idx = -1;
    
    for (i8 i = 0; check[check_x][i] != -1; ++i) {
        // We can't drop the axis puyo on the 14th row
        if (heights[check[check_x][i]] > 12) {
            return false;
        }

        // Checks if we have to do a floor kick
        if (heights[check[check_x][i]] == 12 && height_12_idx == -1) {
            height_12_idx = check[check_x][i];
        }
    }

    // If we don't have to do a floor kick, return true
    if (height_12_idx == -1) {
        return true;
    }

    // If we can do a 180 rotation at the spawn point, return true
    if (heights[1] > 11 && heights[3] > 11) {
        return true;
    }

    // Check if we can floor kick
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