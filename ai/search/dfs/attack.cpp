#include "attack.h"

namespace dfs
{

namespace attack
{

// Searches for all possible attacks
Result search(
    Field field,
    cell::Queue queue,
    bool detect,
    i32 frame_delay,
    i32 thread_count
)
{
    if (queue.size() < 2) {
        return Result();
    }

    Result result = Result();

    // Creates root
    Node root = Node {
        .field = field,
        .score = 0,
        .frame = 0
    };

    // Generates placements
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
                    .attack_max = attack::Data(),
                    .attacks = std::vector<attack::Data>(),
                    .attacks_ac = std::vector<attack::Data>(),
                    .attacks_detect = std::vector<attack::Data>()
                };

                candidate.attacks.reserve(512);
                candidate.attacks_detect.reserve(512);

                // Creates child node
                auto child = root;

                child.field.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.field.pop();

                // Checks for death
                if (child.field.get_height(2) > 11) {
                    continue;
                }

                // Gets chain score
                auto chain = chain::get_score(mask_pop);

                if (chain.count > 0) {
                    // Pushes attack to the candidate
                    auto attack = attack::Data {
                        .count = chain.count,
                        .score = chain.score,
                        .score_total = chain.score,
                        .frame = 0,
                        .frame_real = root.field.get_drop_pair_frame(placement.x, placement.r),
                        .all_clear = child.field.is_empty(),
                        .redundancy = INT32_MAX,
                        .link = eval::get_link(child.field),
                        .parent = root.field,
                        .result = child.field
                    };

                    candidate.attacks.push_back(attack);

                    // Checks for all clear
                    if (attack.all_clear) {
                        candidate.attacks_ac.push_back(attack);
                    }

                    // Updates max attack
                    candidate.attack_max = attack;
                }

                // Accumulates stats
                child.score += chain.score;
                child.frame += root.field.get_drop_pair_frame(placement.x, placement.r) + chain.count * 2 + frame_delay;

                // Continues searching
                attack::dfs(
                    child,
                    queue,
                    candidate,
                    1,
                    detect,
                    frame_delay
                );

                // Dead end
                if (candidate.attacks.empty()) {
                    continue;
                }

                // Locks mutex and pushes candidate
                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(candidate);
                }
            }
        });
    }

    // Joins all threads
    for (auto& t : threads) {
        t.join();
    }

    return result;
};

// Depth first search
void dfs(
    Node& node,
    cell::Queue& queue,
    Candidate& candidate,
    i32 depth,
    bool detect,
    i32 frame_delay
)
{
    // Generates possible placements
    auto placements = move::generate(node.field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        // Creates child
        auto child = node;
        child.field.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.field.pop();

        // Checks for death
        if (child.field.get_height(2) > 11) {
            continue;
        }

        // Gets chain score
        auto chain = chain::get_score(mask_pop);

        if (chain.count > 0) {
            // Pushes attack
            auto attack = attack::Data {
                .count = chain.count,
                .score = chain.score,
                .score_total = child.score + chain.score,
                .frame = child.frame,
                .frame_real = child.frame + node.field.get_drop_pair_frame(placements[i].x, placements[i].r),
                .all_clear = child.field.is_empty(),
                .redundancy = INT32_MAX,
                .link = eval::get_link(child.field),
                .parent = node.field,
                .result = child.field
            };

            candidate.attacks.push_back(attack);

            // Checks all clear
            if (attack.all_clear) {
                candidate.attacks_ac.push_back(attack);
            }

            // Updates max attack
            candidate.attack_max = std::max(
                candidate.attack_max,
                attack,
                attack::cmp_main
            );
        }

        // Updates stats
        child.score += chain.score;
        child.frame += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2 + frame_delay;

        if (depth + 1 < queue.size()) {
            // Continues searching if we are not at the end of the queue
            attack::dfs(
                child,
                queue,
                candidate,
                depth + 1,
                detect,
                frame_delay
            );
        }
        else {
            // If we are at the end of the queue and we want to search further
            if (detect) {
                quiet::search(child.field, 1, 2, [&] (quiet::Result q) {
                    auto plan_pop = q.plan;
                    plan_pop.pop();

                    candidate.attacks_detect.push_back(attack::Data {
                        .count = q.chain.count,
                        .score = q.chain.score,
                        .score_total = child.score + q.chain.score,
                        .frame = child.frame + q.plan.get_height(q.x) - child.field.get_height(q.x),
                        .frame_real = child.frame + 1 + q.plan.get_height(q.x) - child.field.get_height(q.x),
                        .all_clear = false,
                        .redundancy = INT32_MAX,
                        .link = eval::get_link(plan_pop),
                        .parent = child.field,
                        .result = plan_pop
                    });
                });
            }
        }
    }
};

};

};