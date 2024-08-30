#include "quiet.h"

namespace Quiet
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

    i8 x_min;
    i8 x_max;
    Quiet::get_bound(heights, -1, x_min, x_max);

    Quiet::generate(
        field,
        x_min,
        x_max,
        -1,
        drop,
        [&] (i8 x, i8 p, i8 need, i8 dir) {
            if (dir != 0) {
                return;
            }

            auto plan = field;

            for (i32 i = 0; i < need; ++i) {
                plan.data[p].set_bit(x, heights[x] + i);
            }

            auto sim = plan;
            auto sim_mask = sim.pop();
            auto sim_chain = Chain::get_score(sim_mask);

            if (sim_chain.count > 1) {
                callback(Result {
                    .chain = sim_chain.count,
                    .score = sim_chain.score,
                    .x = x,
                    .depth = 0,
                    .plan = plan,
                    .remain = sim
                });
            }

            if (depth < 2) {
                return;
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

            u8 plan_heights[6];
            plan.get_heights(plan_heights);

            i8 plan_x_min;
            i8 plan_x_max;
            Quiet::get_bound(plan_heights, x, plan_x_min, plan_x_max);
            
            Quiet::dfs(
                sim,
                plan,
                plan_x_min,
                plan_x_max,
                x,
                sim_chain.count,
                depth - 1,
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

    Quiet::generate(
        field,
        x_min,
        x_max,
        x_ban,
        2,
        [&] (i8 x, i8 p, i8 need, i8 dir) {
            if (i32(pre_heights[x]) + need + (x == 2) > 12) {
                return;
            }

            auto plan = pre;
            
            switch (dir)
            {
            case 0:
                for (i32 i = 0; i < need; ++i) {
                    plan.data[p].set_bit(x, pre_heights[x] + i);
                }
                break;
            case 1:
                plan.data[p].set_bit(x, pre_heights[x]);
                plan.data[p].set_bit(x + 1, pre_heights[x + 1]);
                break;
            case -1:
                plan.data[p].set_bit(x, pre_heights[x]);
                plan.data[p].set_bit(x - 1, pre_heights[x - 1]);
                break;
            }

            if (plan.data[p].get_mask_group_4(x, pre_heights[x]).get_count() > 3) {
                return;
            }

            u8 plan_heights[6];
            plan.get_heights(plan_heights);

            i8 plan_x_min;
            i8 plan_x_max;
            Quiet::get_bound(plan_heights, x_ban, plan_x_min, plan_x_max);

            if (x_ban + 1 < plan_x_min || plan_x_max + 1 < x_ban) {
                return;
            }

            auto sim = plan;
            auto sim_mask = sim.pop();

            if (sim_mask.get_size() > pre_count) {
                auto sim_chain = Chain::get_score(sim_mask);

                callback(Result {
                    .chain = sim_chain.count,
                    .score = sim_chain.score,
                    .x = x_ban,
                    .depth = depth,
                    .plan = plan,
                    .remain = sim
                });

                if (depth > 1) {
                    Quiet::dfs(
                        sim,
                        plan,
                        plan_x_min,
                        plan_x_max,
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
    std::function<void(i8, i8, i8, i8)> callback
)
{
    u8 heights[6];
    field.get_heights(heights);

    for (i8 x = x_min; x <= x_max; ++x) {
        if (x == x_ban) {
            continue;
        }

        bool expand_r = x < 5 && x + 1 != x_ban && heights[x] == heights[x + 1] && !(x == 1 && heights[2] == 11);
        bool expand_l = x > 0 && x - 1 != x_ban && heights[x] == heights[x - 1] && !(x == 3 && heights[2] == 11);

        i32 drop_max = std::min(drop, 12 - i32(heights[x]) - i32(x == 2));

        if (drop_max <= 0) {
            continue;
        }

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            auto copy = field;
            bool call = false;

            for (u8 i = 0; i < drop_max; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    call = true;
                    callback(x, p, i + 1, 0);
                    break;
                }
            }

            if (drop_max > 1 && call) {
                if (expand_r && field.data[p].get_bit(x + 1, heights[x] - 1) == 0) {
                    callback(x, p, drop_max, 1);
                }

                if (expand_l && field.data[p].get_bit(x - 1, heights[x] - 1) == 0) {
                    callback(x, p, drop_max, -1);
                }
            }
        }
    }
};

void get_bound(
    u8 heights[6],
    i8 x_ban,
    i8& x_min,
    i8& x_max
)
{
    x_min = 0;

    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11 && i != x_ban) {
            x_min = i + 1;
            break;
        }
    }

    x_max = 5;

    for (i8 i = 2; i < 6; ++i) {
        if (heights[i] > 11 && i != x_ban) {
            x_max = i - 1;
            break;
        }
    }
};

};