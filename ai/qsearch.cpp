#include "qsearch.h"

namespace Qsearch
{

void search(
    Field& field,
    i32 depth,
    i32 drop,
    std::function<void(Result)> callback
)
{
    u8 heights[6];
    field.get_heights(heights);

    i8 x_min = 0;
    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11) {
            x_min = i + 1;
            break;
        }
    }

    i8 x_max = 5;
    for (i8 i = 2; i < 6; ++i) {
        if (heights[i] > 11) {
            x_max = i - 1;
            break;
        }
    }

    Qsearch::generate(
        field,
        x_min,
        x_max,
        -1,
        drop,
        [&] (i8 x, i8 p, i32 need) {
            auto copy = field;

            for (i32 i = 0; i < need; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);
            }

            auto sim = copy;
            auto sim_mask = sim.pop();
            auto sim_chain = Chain::get_score(sim_mask);

            if (sim_chain.count > 1) {
                callback(Result {
                    .chain = sim_chain.count,
                    .score = sim_chain.score,
                    .x = x,
                    .plan = copy
                });
            }

            u8 sim_heights[6];
            sim.get_heights(sim_heights);

            bool extendable = false;
            for (i32 i = x_min; i <= x_max; ++i) {
                if (i == x) {
                    continue;
                }

                if (heights[i] != sim_heights[i]) {
                    extendable = true;
                    break;
                }
            }

            if (!extendable) {
                return;
            }
            
            Qsearch::dfs(
                sim,
                copy,
                x_min,
                x_max,
                x,
                sim_chain.count,
                depth,
                callback
            );
        }
    );
};

void dfs(
    Field& field,
    Field& pre,
    i8 x_min,
    i8 x_max,
    i8 x_ban,
    i32 pre_count,
    i32 depth,
    std::function<void(Result)>& callback
)
{
    u8 pre_heights[6];
    pre.get_heights(pre_heights);

    Qsearch::generate(
        field,
        x_min,
        x_max,
        x_ban,
        2,
        [&] (i8 x, i8 p, i32 need) {
            if (i32(pre_heights[x]) + need + (x == 2) > 12) {
                return;
            }

            auto copy = pre;
            
            for (i32 i = 0; i < need; ++i) {
                copy.data[p].set_bit(x, pre_heights[x] + i);
            }

            if (copy.data[p].get_mask_group_4(x, pre_heights[x]).get_count() > 3) {
                return;
            }

            auto sim = copy;
            auto sim_mask = sim.pop();

            if (sim_mask.get_size() > pre_count) {
                auto sim_chain = Chain::get_score(sim_mask);

                callback(Result {
                    .chain = sim_chain.count,
                    .score = sim_chain.score,
                    .x = x_ban,
                    .plan = copy
                });

                if (depth > 1) {
                    Qsearch::dfs(
                        sim,
                        copy,
                        x_min,
                        x_max,
                        x_ban,
                        sim_chain.count,
                        depth - 1,
                        callback
                    );
                }
            }
        }
    );
};

void generate(
    Field& field,
    i8 x_min,
    i8 x_max,
    i8 x_ban,
    i32 drop,
    std::function<void(i8, i8, i32)> callback
)
{
    u8 heights[6];
    field.get_heights(heights);

    for (i8 x = x_min; x <= x_max; ++x) {
        if (x == x_ban) {
            continue;
        }

        u8 drop_max = std::min(drop, 12 - heights[x] - (x == 2));

        if (drop_max == 0) {
            continue;
        }

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            auto copy = field;

            copy.data[p].set_bit(x, heights[x]);

            i32 need = 0;
            i32 mask_count = copy.data[p].get_mask_group_4(x, heights[x]).get_count();

            if (mask_count <= 1) {
                for (i32 i = 0; i < 2; ++i) {
                    copy.data[p].set_bit(x, heights[x] + 1 + i);
                }

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    need = 3;
                }
            }
            else {
                need = std::max(0, 4 - mask_count) + 1;
            }

            if (need == 0 || need > drop_max) {
                continue;
            }

            callback(x, p, need);
        }
    }
};

};