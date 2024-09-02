#include "attack.h"

namespace Attack
{

Result search(
    Field field,
    std::vector<Cell::Pair> queue,
    bool deep,
    i32 frame_delay,
    i32 thread_count
)
{
    if (queue.size() < 2) {
        return Result();
    }

    Result result = Result();

    Node root = Node {
        .field = field,
        .score = 0,
        .frame = 0
    };

    auto placements = Move::generate(field, queue[0].first == queue[0].second);

    std::mutex mtx;
    std::vector<std::thread> threads;

    for (i32 t = 0; t < thread_count; ++t) {
        threads.emplace_back([&] () {
            while (true)
            {
                Move::Placement placement;

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    if (placements.get_size() < 1) {
                        break;
                    }
                    placement = placements[placements.get_size() - 1];
                    placements.pop();
                }

                Candidate candidate = Candidate {
                    .placement = placement,
                    .attack_max = Attack::Data(),
                    .attacks = std::vector<Attack::Data>(),
                    .attacks_ac = std::vector<Attack::Data>(),
                    .attacks_detect = std::vector<Attack::Data>()
                };

                candidate.attacks.reserve(512);
                candidate.attacks_detect.reserve(512);

                auto child = root;

                child.field.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.field.pop();

                if (child.field.get_height(2) > 11) {
                    continue;
                }

                auto chain = Chain::get_score(mask_pop);

                if (chain.count > 0) {
                    auto attack = Attack::Data {
                        .count = chain.count,
                        .score = chain.score,
                        .score_total = chain.score,
                        .frame = 0,
                        .frame_real = root.field.get_drop_pair_frame(placement.x, placement.r),
                        .redundancy = INT32_MAX,
                        .link = Eval::get_link(child.field),
                        .all_clear = child.field.is_empty(),
                        .parent = root.field,
                        .result = child.field
                    };

                    candidate.attacks.push_back(attack);

                    if (attack.all_clear) {
                        candidate.attacks_ac.push_back(attack);
                    }

                    candidate.attack_max = attack;
                }

                child.score += chain.score;
                child.frame += root.field.get_drop_pair_frame(placement.x, placement.r) + chain.count * 2 + frame_delay;

                Attack::dfs(
                    child,
                    queue,
                    candidate,
                    1,
                    deep,
                    frame_delay
                );

                if (candidate.attacks.empty()) {
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(candidate);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
};

void dfs(
    Node& node,
    std::vector<Cell::Pair>& queue,
    Candidate& candidate,
    i32 depth,
    bool deep,
    i32 frame_delay
)
{
    auto placements = Move::generate(node.field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        auto child = node;
        child.field.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        auto chain = Chain::get_score(mask_pop);

        if (chain.count > 0) {
            auto attack = Attack::Data {
                .count = chain.count,
                .score = chain.score,
                .score_total = child.score + chain.score,
                .frame = child.frame,
                .frame_real = child.frame + node.field.get_drop_pair_frame(placements[i].x, placements[i].r),
                .redundancy = INT32_MAX,
                .link = Eval::get_link(child.field),
                .all_clear = child.field.is_empty(),
                .parent = node.field,
                .result = child.field
            };

            candidate.attacks.push_back(attack);

            if (attack.all_clear) {
                candidate.attacks_ac.push_back(attack);
            }

            candidate.attack_max = std::max(
                candidate.attack_max,
                attack,
                Attack::cmp_main
            );
        }

        child.score += chain.score;
        child.frame += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2 + frame_delay;

        if (depth + 1 < queue.size()) {
            Attack::dfs(
                child,
                queue,
                candidate,
                depth + 1,
                deep,
                frame_delay
            );
        }
        else {
            if (deep) {
                Quiet::search(child.field, 1, 2, [&] (Quiet::Result q) {
                    auto plan_pop = q.plan;
                    plan_pop.pop();

                    candidate.attacks_detect.push_back(Attack::Data {
                        .count = q.chain,
                        .score = q.score,
                        .score_total = child.score + q.score,
                        .frame = child.frame + q.plan.get_height(q.x) - child.field.get_height(q.x),
                        .frame_real = child.frame + 1 + q.plan.get_height(q.x) - child.field.get_height(q.x),
                        .redundancy = INT32_MAX,
                        .link = Eval::get_link(plan_pop),
                        .all_clear = false,
                        .parent = child.field,
                        .result = plan_pop
                    });
                });
            }
        }
    }
};

};