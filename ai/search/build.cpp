#include "build.h"

namespace Build
{

bool Thread::search(Field field, Cell::Queue queue, std::vector<Eval::Weight> w)
{
    if (this->thread != nullptr) {
        return false;
    }

    this->clear();

    this->thread = new std::thread([&] (Field f, Cell::Queue q, std::vector<Eval::Weight> w_list) {
        for (auto& weight : w_list) {
            this->results.push_back(Build::search(f, q, weight));
        }
    }, field, queue, w);

    return true;
};

std::vector<Result> Thread::get()
{
    if (this->thread == nullptr) {
        return {};
    }

    this->thread->join();

    auto result = this->results;

    this->clear();

    return result;
};

void Thread::clear()
{
    if (this->thread != nullptr) {
        if (this->thread->joinable()) {
            this->thread->join();
        }
        delete this->thread;
    }

    this->thread = nullptr;
    this->results.clear();
};

Result search(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 thread_count)
{
    if (queue.size() < 2) {
        return Result();
    }

    Result result = Result();

    Node root = Node {
        .field = field,
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
                    .eval = Eval::Result(),
                    .eval_fast = INT32_MIN
                };

                auto child = root;

                child.field.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.field.pop();

                if (child.field.get_height(2) > 11) {
                    continue;
                }

                child.tear += root.field.get_drop_pair_frame(placement.x, placement.r) - 1;
                child.waste += mask_pop.get_size();

                candidate.eval_fast = Eval::evaluate(child.field, child.tear, child.waste, w).value;

                candidate.eval = Build::dfs(child, queue, w, 1);

                if (candidate.eval.value == INT32_MIN) {
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

Eval::Result dfs(Node& node, std::vector<Cell::Pair>& queue, Eval::Weight& w, i32 depth)
{
    auto result = Eval::Result();

    auto placements = Move::generate(node.field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        auto child = node;
        child.field.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        child.tear += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) - 1;
        child.waste += mask_pop.get_size();

        Eval::Result eval = Eval::Result();

        if (depth + 1 < queue.size()) {
            eval = Build::dfs(child, queue, w, depth + 1);
        }
        else {
            eval = Eval::evaluate(child.field, child.tear, child.waste, w);
        }

        if (eval.value > result.value) {
            result.value = eval.value;
            result.plan = eval.plan;
        }

        if (eval.q > result.q) {
            result.q = eval.q;
        }
    }

    return result;
};

};