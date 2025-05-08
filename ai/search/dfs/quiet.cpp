#include "quiet.h"

namespace dfs
{

namespace quiet
{

// Searches all the potential chain extensions of the field
// This function behaves like quiescence search in chess engines
// We continue dropping key puyos until there aren't any possible chains left, then accumulate the results to find the potential chains of the field
void search(
    Field& field,
    i32 depth,
    i32 drop,
    std::function<void(Result)> callback
)
{
    u8 heights[6];
    field.get_heights(heights);

    auto [x_min, x_max] = quiet::get_bound(heights);

    // Drops puyo until a chain is triggered for all columns and colors
    quiet::generate(
        field,
        x_min,
        x_max,
        -1,
        drop,
        [&] (i8 x, i8 p, i8 need, i8 dir) {
            // We don't drop puyos horizontally on the 1st time
            if (dir != 0) {
                return;
            }

            // Drops puyo
            auto plan = field;

            for (i32 i = 0; i < need; ++i) {
                plan.data[p].set_bit(x, heights[x] + i);
            }

            // Pops field
            auto sim = plan;
            auto sim_mask = sim.pop();
            auto sim_chain = chain::get_score(sim_mask);

            // Checks for callback
            if (sim_chain.count > 1) {
                callback(Result {
                    .chain = chain::Score {
                        .count = sim_chain.count,
                        .score = sim_chain.score
                    },
                    .x = x,
                    .plan = plan,
                    .remain = sim
                });
            }

            if (depth < 2) {
                return;
            }

            // Checks if this chain is extendable
            // A chain is extendable if it affects other columns outside the column that we dropped the key puyos
            //
            // Ex:
            // 1. If we drop a blue puyo on the 2nd column, we will change the heights of the 1st columns
            //    We can then drop a red puyo on the 1st column to extend the chain:
            // ......    R.....
            // B.....    B.....
            // B..... -> B.....
            // B.....    BB....
            // RRR...    RRR...
            //
            // 2. If we drop a blue puyo on the 1st column, we won't change the heights of any other columns, thus we can't extend this chain:
            // ......    B.....
            // B.....    B.....
            // B..... -> B.....
            // B.....    B.....
            // RRR...    RRR...
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

            // Continues searching
            quiet::dfs(
                sim,
                plan,
                x,
                sim_chain.count,
                depth - 1,
                callback
            );
        }
    );
};

// Continues dropping puyos to extend chains
void dfs(
    Field& field,
    Field& root,
    i8 x_ban,
    i32 pre_chain,
    i32 depth,
    std::function<void(Result)>& callback
)
{
    u8 root_heights[6];
    root.get_heights(root_heights);

    auto [x_min, x_max] = quiet::get_bound(root_heights);

    quiet::generate(
        field,
        x_min,
        x_max,
        x_ban,
        2,
        [&] (i8 x, i8 p, i8 need, i8 dir) {
            // Checks if we dropped the key puyos above the 12th row or killed ourself
            if (i32(root_heights[x]) + need + (x == 2) > 12) {
                return;
            }

            // Drops puyos
            auto plan = root;

            switch (dir)
            {
            case 0:
                // Dropping vertically
                for (i32 i = 0; i < need; ++i) {
                    plan.data[p].set_bit(x, root_heights[x] + i);
                }
                break;
            case 1:
                // Expanding to the right
                plan.data[p].set_bit(x, root_heights[x]);
                plan.data[p].set_bit(x + 1, root_heights[x + 1]);
                break;
            case -1:
                // Expanding to the left
                plan.data[p].set_bit(x, root_heights[x]);
                plan.data[p].set_bit(x - 1, root_heights[x - 1]);
                break;
            }

            // Checks for chain cuts
            if (plan.data[p].get_mask_group_4(x, root_heights[x]).get_count() > 3) {
                return;
            }

            // Checks if we can reach the trigger point after dropping the key puyos
            //
            // Ex:
            // ....Y.    ...BY.  (12th row)
            // .BBYY. -> .BBYY.
            // ###BY#    ###BY#
            // ######    ######
            //
            // We can place a blue puyo on the 4th column to extend this chain
            // However, realistically, to trigger the chain, we first have to place the blue puyo first, then the yellow puyo last
            // But dropping the blue puyo on the 4th column blocks the way for the yellow puyo to reach the 5th column
            // So this chain is actually impossible and we have to prune this
            //
            // Notes:
            // In reality, sometimes we CAN reach the 5th column even if we drop the blue puyo on the 4th column
            // However, since quiet::search() is only an estimation for the field's potential chain, we want to eliminate as much risk as possible
            u8 plan_heights[6];
            plan.get_heights(plan_heights);

            if (!quiet::is_reachable(plan_heights, x_ban)) {
                return;
            }

            // Simulates chain
            auto sim = plan;
            auto sim_mask = sim.pop();

            // If we extended the chain successfully
            if (sim_mask.get_size() > pre_chain) {
                // Callback
                callback(Result {
                    .chain = chain::get_score(sim_mask),
                    .x = x_ban,
                    .plan = plan,
                    .remain = sim
                });

                // Continues searching
                if (depth > 1) {
                    quiet::dfs(
                        sim,
                        plan,
                        x_ban,
                        sim_mask.get_size(),
                        depth - 1,
                        callback
                    );
                }
            }
        }
    );
};

// Gets the dropping bound
std::pair<i8, i8> get_bound(u8 heights[6])
{
    i8 x_min = 2;
    i8 x_max = 2;

    for (i8 x = 3; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        x_max += 1;
    }

    for (i8 x = 1; x > -1; --x) {
        if (heights[x] > 11) {
            break;
        }

        x_min -= 1;
    }

    return { x_min, x_max };
};

// Checks if a column is reachable
bool is_reachable(u8 heights[6], i8 x)
{
    assert(x >= 0 && x < 6);

    for (i8 i = 3; i < x; ++i) {
        if (heights[i] > 11) {
            return false;
        }
    }

    for (i8 i = 1; i > x; --i) {
        if (heights[i] > 11) {
            return false;
        }
    }

    return true;
};

// Finds dropping positions that may trigger a chain
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

    // Map for checking transpositions while dropping pairs horizontally
    //
    // Ex:
    // 1. We can drop 1 green on the 1st column then drop another green on the 2nd column
    // ......    ......    ......
    // ...... -> G..... -> GG....
    // GG....    GG....    GG....
    //
    // 2. We can also drop 1 green on the 2nd column then drop another on the 1st column to reach the same result
    // ......    ......    ......
    // ...... -> .G.... -> GG....
    // GG....    GG....    GG....
    //
    // Even though we only drop 1 green-green pair on the 1st and 2nd columns, we have to call the callback() function twice
    // Using this map, we can check for duplicates and remove them
    bool horizontal_checked[5][cell::COUNT - 1] = { false };

    for (i8 x = x_min; x <= x_max; ++x) {
        if (x == x_ban) {
            continue;
        }

        // Checks if we can drop puyos horizontally left and right
        // We can only extend the chain horizontally if the surface is flat
        //
        // Ex:
        // 1. We can extend this chain horizontally because the 3rd and 4th columns have the same heights:
        // ......    ......
        // ......    ......
        // .R.... -> .RBB..
        // .BB#..    .BB#..
        // RRR#..    RRR#..
        //
        // 2. We can't extend this chain horizontally because the 3rd and 4th columns have different heights
        //    We are forced to drop puyos vertically on the 3rd column:
        // ......    ......
        // ......    ..B...
        // .R.... -> .RB...
        // .BB...    .BB...
        // RRR...    RRR...
        bool expand_r = (x < 5) && (x + 1 != x_ban) && (heights[x] == heights[x + 1]) && !(x == 1 && heights[2] == 11);
        bool expand_l = (x > 0) && (x - 1 != x_ban) && (heights[x] == heights[x - 1]) && !(x == 3 && heights[2] == 11);

        // Finds the maximum amount of puyo blobs that can be drop
        i32 drop_max = std::min(drop, 12 - i32(heights[x]));

        if (drop_max <= 0) {
            continue;
        }

        // For every color
        for (u8 p = 0; p < cell::COUNT - 1; ++p) {
            auto copy = field;
            i8 dropped = 0;

            // Continues dropping puyo blobs until we trigger a chain
            for (i8 i = 0; i < drop_max; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    callback(x, p, i + 1, 0);
                    dropped = i + 1;
                    break;
                }
            }

            // Tries dropping horizontally
            if (dropped > 1) {
                // Extends right
                if (expand_r && !horizontal_checked[x][p]) {
                    callback(x, p, drop_max, 1);
                    horizontal_checked[x][p] = true;
                }

                // Extends left
                if (expand_l && !horizontal_checked[x - 1][p]) {
                    callback(x, p, drop_max, -1);
                    horizontal_checked[x - 1][p] = true;
                }
            }
        }
    }
};

};

};