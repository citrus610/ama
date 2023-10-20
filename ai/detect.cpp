#include "detect.h"

namespace Detect
{

void detect(Field& field, std::function<void(Result)> callback, i32 drop, i32 drop_well)
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

    auto m12 = field.get_mask().get_mask_12();
    auto mask_empty = FieldBit();
    mask_empty.data = ~m12.data;

    for (i8 x = min_x; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        bool well = Detect::is_well(heights, x);

        u8 drop_max = std::min(
            well ? drop_well : drop,
            12 - heights[x] - (x == 2)
        );

        if (drop_max == 0) {
            continue;
        }

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            auto copy = field;

            copy.data[p].set_bit(x, heights[x]);
            auto need = 4 - copy.data[p].get_mask_group_4(x, heights[x]).get_count();

            if (need > drop_max - 1) {
                continue;
            }

            for (i32 i = 0; i < need; ++i) {
                copy.data[p].set_bit(x, heights[x] + i + 1);
            }

            auto pop = copy;
            auto copy_mask = pop.pop();

            if (copy_mask.get_size() > 1) {
                auto copy_chain = Chain::get_score(copy_mask);

                i32 extensibility = -1;
                if (!well) {
                    extensibility = ((copy_mask[0].data[p] & field.data[p]).get_expand() & mask_empty).get_mask_12().get_count();
                }

                callback(Result {
                    .chain = copy_mask.get_size(),
                    .score = copy_chain.score,
                    .y = heights[x],
                    .extensibility = extensibility,
                    .plan = pop
                });
            }

            if (copy_mask.get_size() > 0) {
                for (i8 dtx = -2; dtx <= 2; ++dtx) {
                    if (dtx == 0) {
                        continue;
                    }

                    i8 x_k = x + dtx;

                    if (x_k < 0 || x_k > 5 || heights[x_k] > 11 || (x_k == 2 && heights[x_k] > 10) || pop.get_height(x_k) == heights[x_k]) {
                        continue;
                    }

                    for (u8 k = 0; k < Cell::COUNT - 1; ++k) {
                        auto copy_copy = copy;

                        copy_copy.data[k].set_bit(x_k, heights[x_k]);

                        if (copy_copy.data[k].get_mask_group_4(x_k, heights[x_k]).get_count() >= 4) {
                            continue;
                        }

                        auto copy_copy_mask = copy_copy.pop();

                        if (copy_copy_mask.get_size() > copy_mask.get_size()) {
                            auto copy_copy_chain = Chain::get_score(copy_copy_mask);

                            auto copy_mask_empty = m12;
                            copy_mask_empty.set_bit(x_k, heights[x_k]);
                            copy_mask_empty.data = ~copy_mask_empty.data;

                            i32 copy_extensibility = -1;
                            if (!well) {
                                copy_extensibility = ((copy_mask[0].data[p] & field.data[p]).get_expand() & copy_mask_empty).get_mask_12().get_count();
                            }

                            callback(Result {
                                .chain = copy_copy_mask.get_size(),
                                .score = copy_copy_chain.score,
                                .y = heights[x],
                                .extensibility = copy_extensibility,
                                .plan = copy_copy
                            });
                        }
                    }
                }
            }
        }
    }
};

void detect_fast(Field& field, std::function<void(Result)> callback)
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

        if (x == 2 && heights[2] >= 11) {
            continue;
        }

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            copy.data[p].set_bit(x, heights[x]);

            if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() < 4) {
                break;
            }

            auto mask = copy.pop();
            auto chain = Chain::get_score(mask);

            if (mask.get_size() < 2) {
                continue;
            }

            callback(Result {
                .chain = chain.count,
                .score = chain.score,
                .y = heights[x],
                .plan = copy
            });
        }
    }
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

};