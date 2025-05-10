#include "form.h"

namespace beam
{

namespace form
{

// Human pattern matching
i32 evaluate_2(Field& field, u8 height[6], const Data& pattern)
{
    i32 result = 0;
    const i32 error = -100;

    auto map = std::array<cell::Type, AREA>();
    map.fill(cell::Type::NONE);

    // Makes map
    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            if (height[x] <= y) {
                break;
            }

            // Gets the cell's group index
            i32 index = pattern.form[y][x];

            if (index == 0) {
                continue;
            }

            // Probes map
            if (map[index] == cell::Type::NONE) {
                map[index] = field.get_cell(x, y);
            }
            else if (map[index] != field.get_cell(x, y) && pattern.matrix[index][index] > 0) {
                return error;
            }

            // Increases result
            result += pattern.matrix[index][index];
        }
    }

    // Checks if map is valid
    for (i32 i = 1; i < pattern.groups; ++i) {
        if (map[i] == cell::Type::NONE) {
            continue;
        }

        for (i32 k = i + 1; k <= pattern.groups; ++k) {
            if (map[k] == cell::Type::NONE) {
                continue;
            }

            // Gets indices' relationship
            i32 rel = pattern.matrix[i][k];

            if (rel == 0) {
                continue;
            }
            else if (rel < 0) {
                if (map[i] == map[k]) {
                    return error;
                }
            }
            else {
                if (map[i] != map[k]) {
                    return error;
                }

                result += pattern.matrix[i][k];
            }
        }
    }

    return result;
};

i32 evaluate(Field& field, u8 height[6], const Data& pattern)
{
    i32 result = 0;
    const i32 error = -100;

    for (i8 x0 = 0; x0 < 6; ++x0) {
        for (i8 y0 = 0; y0 < HEIGHT; ++y0) {
            if (height[x0] <= y0) {
                break;
            }

            if (pattern.form[y0][x0] == 0) {
                continue;
            }

            for (i8 x1 = x0; x1 < 6; ++x1) {
                for (i8 y1 = 0; y1 < HEIGHT; ++y1) {
                    if (height[x1] <= y1) {
                        break;
                    }

                    if (pattern.form[y1][x1] == 0) {
                        continue;
                    }

                    if (x0 == x1 && y0 >= y1) {
                        continue;
                    }

                    i8 field_rel = i8(field.get_cell(x0, y0) == field.get_cell(x1, y1)) * 2 - 1;
                    i8 pattern_rel = pattern.matrix[pattern.form[y0][x0]][pattern.form[y1][x1]];

                    if (pattern_rel != 0) {
                        if (field_rel * pattern_rel > 0) {
                            result += field_rel * pattern_rel;
                        }
                        else {
                            return error;
                        }
                    }
                }
            }
        }
    }

    return result;
};

Field get_plan(Field& field, const Data& pattern)
{
    Field result = field;

    cell::Type map[HEIGHT * 6];
    memset(map, static_cast<i32>(cell::Type::NONE), HEIGHT * 6 * sizeof(cell::Type));

    for (i32 x = 0; x < 6; ++x) {
        for (i32 y = 0; y < HEIGHT; ++y) {
            if (pattern.form[y][x] == 0 ||
                pattern.matrix[pattern.form[y][x]][pattern.form[y][x]] == 0 ||
                map[pattern.form[y][x]] != cell::Type::NONE) {
                continue;
            }

            auto field_cell = result.get_cell(x, y);

            if (field_cell == cell::Type::NONE) {
                continue;
            }

            map[pattern.form[y][x]] = field_cell;
        }
    }

    for (i32 x = 0; x < 6; ++x) {
        for (i32 y = 0; y < HEIGHT; ++y) {
            if (pattern.form[y][x] == 0) {
                break;
            }

            auto field_cell = result.get_cell(x, y);

            if (field_cell != cell::Type::NONE) {
                continue;
            }

            if (map[pattern.form[y][x]] == cell::Type::NONE && field_cell == cell::Type::NONE) {
                break;
            }

            result.set_cell(x, y, map[pattern.form[y][x]]);
        }
    }

    return result;
};

};

};