#include "search.h"

namespace Search
{

Result search(
    Field field,
    std::vector<Cell::Pair> queue,
    i32 thread_count
)
{
    if (queue.size() <= 2) {
        return Result();
    }

    Result result = Result();

    Node root = Node {
        .field = field,
        .score = 0,
        .frame = 0,
        .tear = 0,
        .waste = 0
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
                    .attacks = std::vector<Attack>(),
                    .attacks_detect = std::vector<Attack>(),
                    .plans = std::vector<Node>(),
                    .plan_fast = Search::Node(),
                    .eval = INT32_MIN,
                    .eval_fast = INT32_MIN
                };

                candidate.attacks.reserve(512);
                candidate.attacks_detect.reserve(512);
                candidate.plans.reserve(512);

                auto child = root;

                child.field.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.field.pop();

                if (child.field.get_height(2) > 11) {
                    continue;
                }

                auto chain = Chain::get_score(mask_pop);

                if (chain.count > 0) {
                    candidate.attacks.push_back(Attack {
                        .count = chain.count,
                        .score = chain.score,
                        .frame = 0,
                        .score_total = chain.score,
                        .frame_total = root.field.get_drop_pair_frame(placement.x, placement.r),
                        .all_clear = child.field.is_empty(),
                        .result = child.field,
                        .eval = INT32_MIN
                    });
                }

                child.score += chain.score;
                child.frame += root.field.get_drop_pair_frame(placement.x, placement.r) + chain.count * 2;
                child.tear += root.field.get_drop_pair_frame(placement.x, placement.r) - 1;
                child.waste += chain.count;

                candidate.plan_fast = child;

                Search::dfs(
                    child,
                    queue,
                    candidate.attacks,
                    candidate.attacks_detect,
                    candidate.plans,
                    1
                );

                if (candidate.attacks.empty() && candidate.plans.empty()) {
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(std::move(candidate));
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
    std::vector<Attack>& attacks,
    std::vector<Attack>& attacks_detect,
    std::vector<Node>& plans,
    i32 depth
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
            attacks.push_back(Attack {
                .count = chain.count,
                .score = chain.score,
                .frame = child.frame,
                .score_total = child.score + chain.score,
                .frame_total = child.frame + node.field.get_drop_pair_frame(placements[i].x, placements[i].r),
                .all_clear = child.field.is_empty(),
                .result = child.field,
                .eval = INT32_MIN
            });
        }

        child.score += chain.score;
        child.frame += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2;
        child.tear += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) - 1;
        child.waste += chain.count;

        if (depth == queue.size() - 2) {
            plans.push_back(child);
        }

        if (depth + 1 < queue.size()) {
            Search::dfs(
                child,
                queue,
                attacks,
                attacks_detect,
                plans,
                depth + 1
            );
        }
        else {
            Detect::detect_fast(child.field, [&] (Detect::Result detect) {
                attacks_detect.push_back(Search::Attack {
                    .count = detect.chain,
                    .score = detect.score,
                    .frame = child.frame + 1,
                    .score_total = child.score + detect.score,
                    .frame_total = child.frame + 2,
                    .all_clear = false,
                    .result = detect.plan,
                    .eval = INT32_MIN
                });
            });
        }
    }
};

};