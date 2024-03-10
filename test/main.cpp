#include <iostream>
#include <fstream>

#include "../ai/ai.h"

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
    to_json(js, Eval::DEFAULT);
    o << std::setw(4) << js << std::endl;
    o.close();
};

static Chain::Score get_score(std::vector<Cell::Pair> queue, Eval::Weight w)
{
    Field field = Field();

    auto best = Chain::Score { 0, 0 };

    for (int i = 0; i < 60; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        auto airesult = AI::think_1p(field, tqueue, w, false);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            return { 0, 0 };
        }

        if (chain.score > best.score) {
            best = chain;

            if (chain.score >= AI::TRIGGER_DEFAULT) {
                break;
            }
        }
    }

    return best;
};

int main()
{
    srand(uint32_t(time(NULL)));

    auto field = Field();

    const char c[13][7] = {
        "......",
        "......",
        "......",
        "......",
        "......",
        ".....R",
        "......",
        ".....R",
        "RR...R",
        ".....R",
        "R.....",
        "RR..BY",
        "GGGBBY"
    };

    // field.from(c);

    field.print();

    // Quiet::search(field, 6, 3, [&] (Quiet::Result q) {
    //     u8 heights[6];
    //     q.plan.get_heights(heights);
    //     heights[q.x] = field.get_height(q.x);

    //     i32 need = q.plan.get_height(q.x) - heights[q.x];

    //     i32 key = 6 - q.depth;

    //     i32 key_s = q.plan.get_count() - field.get_count() - need - key;

    //     q.plan.print();
    //     printf("need: %d\n", need);
    //     printf("key: %d\n", key);
    //     printf("key_s: %d\n", key_s);

    //     std::cin.get();
    // });

    Eval::Weight w;
    save_json_heuristic();
    load_json_heuristic(w);

    const i32 POPULATION_SIZE = 1000;

    std::vector<Cell::Pair> queues[POPULATION_SIZE];
    for (i32 i = 0; i < POPULATION_SIZE; ++i) {
        queues[i] = Cell::create_queue(rand() & 0xFFFF);
    }

    std::atomic<i32> progress = 0;
    std::vector<std::thread> threads;

    std::atomic<i32> map_count[20] = { 0 };
    std::atomic<i32> map_score[POPULATION_SIZE] = { 0 };

    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] (i32 tid) {
            for (i32 i = 0; i < POPULATION_SIZE / 4; ++i) {
                auto queue = queues[tid * POPULATION_SIZE / 4 + i];

                auto score = get_score(queue, w);

                map_count[score.count] += 1;
                map_score[tid * POPULATION_SIZE / 4 + i] = score.score;

                progress += 1;
                printf("\rprogress: %d", progress.load());
            }
        }, t);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::string score_str;

    for (i32 i = 0; i < 20; ++i) {
        score_str += std::to_string(map_count[i].load()) + "\n";
    }

    score_str += "\n";

    for (i32 i = 0; i < POPULATION_SIZE; ++i) {
        score_str += std::to_string(map_score[i].load()) + "\n";
    }

    std::ofstream o("out.txt");
    o << score_str << std::endl;
    o.close();

    return 0;
};