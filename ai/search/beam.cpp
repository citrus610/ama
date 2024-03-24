#include "beam.h"

namespace Beam
{

Layer::Layer()
{
    this->data.clear();
};

void Layer::init(size_t width)
{
    this->clear();

    this->width = width;
    this->data.reserve(this->width);
};

void Layer::clear()
{
    this->data.clear();
};

void Layer::add(Node& node)
{
    if (this->data.size() < this->width) {
        this->data.push_back(node);

        if (this->data.size() == this->width) {
            std::make_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b.eval < a.eval; });
        }

        return;
    }

    if (this->data[0].eval < node.eval) {
        std::pop_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b.eval < a.eval; });
        this->data.back() = node;
        std::push_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b.eval < a.eval; });
    }
};

void Layer::sort()
{
    if (this->data.size() < this->width) {
        std::sort(
            this->data.begin(),
            this->data.end(),
            [&] (Node& a, Node& b) { return b.eval < a.eval; }
        );

        return;
    }

    std::sort_heap(
        this->data.begin(),
        this->data.end(),
        [&] (Node& a, Node& b) { return b.eval < a.eval; }
    );
};

Result search(
    Field field,
    Cell::Queue queue,
    Eval::Weight w,
    i32 trigger,
    size_t width,
    size_t thread
)
{
    Result result = Result();

    if (queue.size() < 2) {
        return result;
    }

    auto root = Node();
    root.field = field;

    Layer layers[2] = { Layer(), Layer() };
    layers[0].init(width);
    layers[1].init(width);

    auto score = Beam::init_candidates(result.candidates, root, queue[0], layers[0], w);

    size_t depth;
    for (depth = 1; depth < queue.size(); ++depth) {
        auto layer_score = Beam::think(queue[depth], layers[(depth - 1) & 1], layers[depth & 1], w);

        if (score.score < layer_score.score) {
            score = layer_score;
        }
    }

    std::mutex mtx;
    std::vector<std::thread> threads;

    // size_t queue_size = (6 * 13 - field.get_count()) / 2 - queue.size() + 4;
    size_t queue_size = 12;

    for (size_t i = 0; i < thread; ++i) {
        auto queue_rng = Beam::generate_queue(queue_size, i);
        queue_rng.insert(queue_rng.begin(), queue.begin(), queue.end());

        threads.emplace_back([&] (Cell::Queue thread_queue, size_t thread_id) {
            auto thread_score = score;
            auto thread_depth = depth;

            Layer thread_layers[2] = { Layer(), Layer() };
            thread_layers[0].init(width);
            thread_layers[1].init(width);
            thread_layers[(depth - 1) & 1] = layers[(depth - 1) & 1];

            for (thread_depth = thread_depth; thread_depth < thread_queue.size(); ++thread_depth) {
                auto layer_score = Beam::think(thread_queue[thread_depth], thread_layers[(thread_depth - 1) & 1], thread_layers[thread_depth & 1], w);

                if (thread_score.score < layer_score.score) {
                    thread_score = layer_score;
                }

                if (thread_score.score >= trigger) {
                    break;
                }
            }

            std::lock_guard<std::mutex> lk(mtx);
            result.candidates[thread_score.index].score += thread_score.score;
        }, queue_rng, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
};

Score expand(
    Node& node,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight& w
)
{
    Score result = Score();

    auto placements = Move::generate(node.field, pair.first == pair.second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        auto child = node;
        child.field.drop_pair(placements[i].x, placements[i].r, pair);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        if (mask_pop.get_size() > 1) {
            auto chain = Chain::get_score(mask_pop);

            if (result.score < chain.score) {
                result.index = child.index;
                result.score = chain.score;
            }
        }

        child.tear += node.field.get_drop_pair_frame(placements[i].x, placements[i].r) - 1;
        child.waste += mask_pop.get_size();

        child.eval = Eval::evaluate(child.field, child.tear, child.waste, w, 1).value;
        layer.add(child);
    }

    return result;
};

Score think(
    Cell::Pair pair,
    Layer& old_layer,
    Layer& new_layer,
    Eval::Weight& w
)
{
    Score result = Score();

    old_layer.sort();

    for (auto& node : old_layer.data) {
        auto score = Beam::expand(node, pair, new_layer, w);

        if (result.score < score.score) {
            result = score;
        }
    }

    old_layer.clear();

    return result;
};

Score init_candidates(
    std::vector<Candidate>& candidates,
    Node& root,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight& w
)
{
    Score result = Score();

    auto placements = Move::generate(root.field, pair.first == pair.second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        candidates.push_back(Candidate {
            .placement = placements[i],
            .score = 0
        });

        auto child = root;
        child.index = i;
        child.field.drop_pair(placements[i].x, placements[i].r, pair);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        if (mask_pop.get_size() > 1) {
            auto chain = Chain::get_score(mask_pop);

            if (result.score < chain.score) {
                result.index = child.index;
                result.score = chain.score;
            }
        }

        child.tear += root.field.get_drop_pair_frame(placements[i].x, placements[i].r) - 1;
        child.waste += mask_pop.get_size();

        child.eval = Eval::evaluate(child.field, child.tear, child.waste, w, 1).value;
        layer.add(child);
    }

    return result;
};

Cell::Queue generate_queue(size_t size, size_t seed)
{
    Cell::Queue result;

    std::vector<Cell::Type> bag;

    for (auto i = 0; i < size * 2; ++i) {
        bag.push_back({Cell::Type(i % (Cell::COUNT - 1))});
    }

    for (auto i = 0; i < size * 2; ++i) {
        std::swap(bag[i], bag[rand() % (size * 2)]);
    }

    for (auto i = 0; i < size; ++i) {
        result.push_back({
            bag[i * 2],
            bag[i * 2 + 1],
        });
    }

    return result;
};

};