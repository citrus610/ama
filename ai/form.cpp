#include "form.h"

namespace Form
{

i32 evaluate(Field& field, u8 height[6], const Data& pattern)
{
    i32 result = 0;
    const i32 error = -10;

    for (i8 x0 = 0; x0 < 6; ++x0) {
        for (i8 y0 = 0; y0 < HEIGHT; ++y0) {
            if (pattern.form[y0][x0] == 0 || height[x0] <= y0) {
                break;
            }

            for (i8 x1 = x0; x1 < 6; ++x1) {
                for (i8 y1 = 0; y1 < HEIGHT; ++y1) {
                    if (pattern.form[y1][x1] == 0 || height[x1] <= y1) {
                        break;
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

    Cell::Type map[HEIGHT * 6];
    memset(map, static_cast<i32>(Cell::Type::NONE), 36 * sizeof(Cell::Type));

    for (i32 x = 0; x < 6; ++x) {
        for (i32 y = 0; y < HEIGHT; ++y) {
            if (pattern.form[y][x] == 0 ||
                pattern.matrix[pattern.form[y][x]][pattern.form[y][x]] == 0 ||
                map[pattern.form[y][x]] != Cell::Type::NONE) {
                continue;
            }

            auto field_cell = result.get_cell(x, y);

            if (field_cell == Cell::Type::NONE) {
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

            if (field_cell != Cell::Type::NONE) {
                continue;
            }

            if (map[pattern.form[y][x]] == Cell::Type::NONE && field_cell == Cell::Type::NONE) {
                break;
            }

            result.set_cell(x, y, map[pattern.form[y][x]]);
        }
    }

    return result;
};

};