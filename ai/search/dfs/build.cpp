#include "build.h"

namespace dfs
{

namespace build
{

// Starts the depth first search
Result search(Field field, cell::Queue queue, eval::Weight w, i32 thread_count)
{
    // We don't search if the input queue is too small
    if (queue.size() < 2) {
        return Result();
    }

    Result result = Result();

    // Initializes root
    Node root = Node {
        .field = field,
        .tear = 0,
        .waste = 0
    };

    // Generates all the possible first placements
    auto placements = move::generate(field, queue[0].first == queue[0].second);

    // Divides the works among the threads
    std::mutex mtx;
    std::vector<std::thread> threads;

    for (i32 t = 0; t < thread_count; ++t) {
        threads.emplace_back([&] () {
            while (true)
            {
                move::Placement placement;

                // Locks mutex and takes 1 placements from the placements list above
                {
                    std::lock_guard<std::mutex> lk(mtx);

                    if (placements.get_size() < 1) {
                        break;
                    }

                    placement = placements[placements.get_size() - 1];
                    placements.pop();
                }

                // Creates candidate
                Candidate candidate = Candidate {
                    .placement = placement,
                    .eval = eval::Result(),
                    .eval_fast = INT32_MIN
                };

                // Updates child node
                auto child = root;

                child.field.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.field.pop();

                // Death
                if (child.field.get_height(2) > 11) {
                    continue;
                }

                // Updates child's stats
                child.tear += root.field.get_drop_pair_frame(placement.x, placement.r) - 1;
                child.waste += mask_pop.get_size();

                candidate.eval_fast = eval::evaluate(child.field, child.tear, child.waste, w).value;

                // Continues searching to evaluate
                candidate.eval = build::dfs(child, queue, w, 1);

                // This child leads to a dead end, so we prune it
                if (candidate.eval.value == INT32_MIN) {
                    continue;
                }

                // Locks mutex and pushes the candidate
                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(std::move(candidate));
                }
            }
        });
    }

    // Joins threads
    for (auto& t : threads) {
        t.join();
    }

    return result;
};

// Depth first search
eval::Result dfs(Node& node, cell::Queue& queue, eval::Weight& w, i32 depth)
{
    auto result = eval::Result();

    // Generates possible placements
    auto placements = move::generate(node.field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        // Creates child node
        auto child = node;
        child.field.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.field.pop();

        // Checks for death
        if (child.field.get_height(2) > 11) {
            continue;
        }

        // Updates stats
        child.tear += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) - 1;
        child.waste += mask_pop.get_size();

        // Evaluates
        eval::Result eval = eval::Result();

        if (depth + 1 < queue.size()) {
            // Continues search if we aren't at the end of the queue
            eval = build::dfs(child, queue, w, depth + 1);
        }
        else {
            // Evaluates if we are at the end of the queue
            eval = eval::evaluate(child.field, child.tear, child.waste, w);
        }

        // Updates the best eval score
        if (eval.value > result.value) {
            result.value = eval.value;
            result.plan = eval.plan;
        }

        // Updates the highest possible chain score from this field
        if (eval.q > result.q) {
            result.q = eval.q;
        }
    }

    return result;
};

};

};