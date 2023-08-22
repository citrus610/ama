#include "detect.h"

namespace Detect
{

void detect(Field& field, std::function<void(Result)> callback, i32 drop_max, i32 drop_min)
{
    u8 heights[6];
    field.get_heights(heights);

    i8 min_x = 0;
    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11) {
            min_x = i + 1;
            break;
        }
    }

    for (i8 x = min_x; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        if (Detect::is_well(heights, x) && heights[x] < 9) {
            continue;
        }

        u8 max_puyo_add = std::min(
            (is_well(heights, x) ? drop_min : drop_max),
            12 - heights[x] - (x == 2)
        );

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            u8 i = 0;
            for (i = 0; i < max_puyo_add; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    break;
                }
            }

            auto plan = copy;

            auto chain_mask_raw = plan.pop();
            auto chain_score_raw = Chain::get_score(chain_mask_raw);

            callback(Result {
                .score = Score {
                    .chain = chain_score_raw,
                    .needed = i + 1,
                    .height = heights[x],
                    .x = x
                },
                .plan = plan
            });

            if (chain_score_raw.count > 0 && chain_score_raw.count <= 8) {
                for (u8 k = 0; k < Cell::COUNT - 1; ++k) {
                    for (i8 delta_x = 0; delta_x < 2; ++delta_x) {
                        i8 x_k = x + delta_x * 2 - 1;
                        
                        if (x_k < 0 || x_k > 5 || heights[x_k] > 11 || (x_k == 2 && heights[x_k] > 10) || heights[x_k] == heights[x]) {
                            continue;
                        }

                        auto copy_copy = copy;

                        copy_copy.drop_puyo(x_k, Cell::Type(k));

                        if (copy_copy.data[k].get_mask_group_4(x_k, copy_copy.get_height(x_k)).get_count() >= 4) {
                            continue;
                        }

                        auto chain_mask = copy_copy.pop();
                        auto chain_score = Chain::get_score(chain_mask);

                        callback(Result {
                            .score = Score {
                                .chain = chain_score,
                                .needed = i + 2,
                                .height = heights[x],
                                .x = x
                            },
                            .plan = copy_copy
                        });
                    }
                }
            }
        }
    }
};

Result detect_fast(Field& field)
{
    Result result;

    u8 heights[6];
    field.get_heights(heights);

    i8 min_x = 0;
    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11) {
            min_x = i + 1;
            break;
        }
    }

    for (i8 x = min_x; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        if (x == 2 && heights[2] == 11) {
            continue;
        }

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            copy.data[p].set_bit(x, heights[x]);

            if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() < 4) {
                break;
            }

            auto chain_mask = copy.pop();
            auto chain_score = Chain::get_score(chain_mask);

            result = std::max(
                result,
                Result {
                    .score = Score {
                        .chain = chain_score,
                        .needed = 1,
                        .height = heights[x],
                        .x = x
                    },
                    .plan = copy
                },
                Detect::cmp_main
            );
        }
    }

    return result;
};

bool is_well(u8 heights[6], i8 x)
{
    if (x == 0) {
        return heights[0] < heights[1];
    }

    if (x == 5) {
        return heights[5] < heights[4];
    }

    return heights[x] < heights[x - 1] && heights[x] < heights[x + 1];
};

bool is_reachable(Field& field, u8 heights[6], i8 x, u8 added, u8 p)
{
    if (!field.data[p].get_bit(x, heights[x] - 1)) {
        return false;
    }

    bool well = false;
    if (x == 0) {
        if (field.data[p].get_bit(1, heights[0])) {
            return false;
        }

        well = heights[0] < heights[1] || (heights[0] == heights[1] && added < 1);
        // well = heights[0] <= heights[1];
    }
    else if (x == 5) {
        if (field.data[p].get_bit(4, heights[5])) {
            return false;
        }

        well = heights[5] < heights[4] || (heights[5] == heights[4] && added < 1);
        // well = heights[5] <= heights[4];
    }
    else {
        if (field.data[p].get_bit(x + 1, heights[x]) || field.data[p].get_bit(x - 1, heights[x])) {
            return false;
        }

        well =
            (heights[x] < heights[x - 1] && heights[x] < heights[x + 1]) ||
            ((heights[x] == heights[x - 1] || heights[x] == heights[x + 1]) && added < 1);
        // well = heights[x] <= heights[x - 1] && heights[x] <= heights[x + 1];
    }

    if (well) {
        return false;
    }

    return true;
};

};