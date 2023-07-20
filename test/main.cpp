#include <iostream>
#include "../ai/ai.h"
#include "../core/core.h"
#include "../ai/eval.h"
#include "../ai/search.h"

void load_json_heuristic(Eval::Weight& h)
{
    std::ifstream file;
    file.open("config.json");
    json js;
    file >> js;
    file.close();
    from_json(js, h);
};

void save_json_heuristic()
{
    std::ifstream f("config.json");
    if (f.good()) {
        return;
    };
    f.close();

    std::ofstream o("config.json");
    json js;
    to_json(js, Eval::DEFAULT_WEIGHT);
    o << std::setw(4) << js << std::endl;
    o.close();
};

struct Score
{
    i32 count = 0;
    i32 score = 0;
};

static Score get_score(std::vector<Cell::Pair> queue, Eval::Weight w)
{
    Field field = Field();

    Score best = Score { 0, 0 };

    for (int i = 0; i < 128; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        AI::Result airesult = AI::think_1p(field, tqueue, w);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            break;
        }

        if (chain.count > 0) {
            best = std::max(
                best,
                Score { .count = chain.count, .score = chain.score },
                [&] (const Score& a, const Score& b) {
                    return a.score < b.score;
                }
            );
        }

        if (chain.score > 10000) {
            break;
        }
    }

    return best;
};

std::vector<Cell::Pair> create_queue()
{
    using namespace std;

    // srand(uint32_t(time(NULL)));

    vector<Cell::Type> bag;
    bag.reserve(256);

    for (int i = 0; i < 64; ++i) {
        for (uint8_t p = 0; p < Cell::COUNT - 1; ++p) {
            bag.push_back(Cell::Type(p));
        }
    }

    for (int t = 0; t < 4; ++t) {
        for (int i = 0; i < 256; ++i) {
            int k = rand() % 256;
            Cell::Type value = bag[i];
            bag[i] = bag[k];
            bag[k] = value;
        }
    }

    vector<Cell::Pair> queue;
    queue.reserve(128);

    for (int i = 0; i < 128; ++i) {
        queue.push_back({ bag[i * 2], bag[i * 2 + 1] });
    }

    return queue;
};

int main()
{
    srand(uint32_t(time(NULL)));

    auto seed = time(NULL);

    i32 score_count = 20;

    std::atomic<i32> map_count[20] = { 0 };
    std::atomic<i32> map_score[score_count] = { 0 };

    std::vector<std::vector<Cell::Pair>> queue_list;
    for (i32 i = 0; i < 1005; ++i) {
        queue_list.push_back(create_queue());
    }

    Eval::Weight w;
    save_json_heuristic();
    load_json_heuristic(w);

    std::mutex mt;

    const i32 sim_count = 1000;
    std::atomic<i32> i = 0;

    std::vector<Score> scores;

    std::vector<std::thread> threads;
    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] () {
            while (i < sim_count)
            {
                auto queue = queue_list[i.load()];
                auto score = get_score(queue, w);
                map_count[score.count] += 1;
                map_score[score.score * score_count / 200000] += 1;
                i++;
                printf("\rprocess: %d/%d", i.load(), sim_count);
                std::lock_guard<std::mutex> lk(mt);
                scores.push_back(score);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    
    std::ofstream o("result.txt");
    o << "AI chain count distribution (" << sim_count << " simulations):\n";
    for (i32 k = 0; k < 20; ++k) {
        o << std::string(2 - std::to_string(k).size(), ' ') << k << " : " << map_count[k].load() << "\n";
    }

    o << "\n";
    o << "AI chain score distribution (" << sim_count << " simulations):\n";
    for (i32 k = 0; k < score_count; ++k) {
        o << std::string(6 - std::to_string(k * 200000 / score_count).size(), ' ') << int(k * 200000 / score_count) << " - "
            << std::string(6 - std::to_string((k + 1) * 200000 / score_count).size(), ' ') << int((k + 1) * 200000 / score_count)
            << " : " << map_score[k].load() << "\n";
    }

    o.close();

    std::string score_str;
    for (auto sc : scores) {
        score_str += std::to_string(sc.score);
        score_str += "\n";
    }

    std::ofstream s("score.txt");
    s << score_str;
    s.close();

    return 0;
};