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

};