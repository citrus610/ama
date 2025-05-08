#include "form.h"

namespace beam
{

namespace form
{

// Human pattern matching
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

// Pattern matching
// But instead of checking the whole field, we only check 2 new puyo blobs at a time
i32 accumulate(Field& field, u8 height[6], i8 x_check[2], i8 y_check[2], const Data& pattern)
{
    i32 result = 0;
    const i32 error = -100;

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            if (height[x] <= y) {
                break;
            }

            if (pattern.form[y][x] == 0) {
                continue;
            }

            auto cell = field.get_cell(x, y);

            for (auto i = 0; i < 2; ++i) {
                if (pattern.form[y_check[i]][x_check[i]] == 0) {
                    continue;
                }

                if (x == x_check[i] && y >= y_check[i]) {
                    continue;
                }

                i8 field_rel = i8(cell == field.get_cell(x_check[i], y_check[i])) * 2 - 1;
                i8 pattern_rel = pattern.matrix[pattern.form[y][x]][pattern.form[y_check[i]][x_check[i]]];

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