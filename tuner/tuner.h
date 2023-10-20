#pragma once

#include "score.h"

namespace Tuner
{

struct SaveData {
    Eval::Weight w;
    i32 count;
    i32 score;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, w, count, score)

void save(std::string id, SaveData s)
{
    std::string name = std::string("data/") + id + std::string(".json");
    std::ofstream o(name);
    json js;
    to_json(js, s);
    o << std::setw(4) << js << std::endl;
    o.close();
};

Eval::Weight constrain(Eval::Weight w)
{
    #define CONSTRAIN_POSITIVE(p) w.p = std::max(0, w.p);
    #define CONSTRAIN_NEGATIVE(p) w.p = std::min(0, w.p);

    CONSTRAIN_POSITIVE(y);

    CONSTRAIN_NEGATIVE(need);
    CONSTRAIN_NEGATIVE(shape);
    CONSTRAIN_NEGATIVE(link_no);
    CONSTRAIN_NEGATIVE(tear);
    CONSTRAIN_NEGATIVE(waste);

    return w;
};

std::pair<Eval::Weight, Eval::Weight> randomize(Eval::Weight w, std::optional<i32> idx = {})
{
    auto w1 = w;
    auto w2 = w;

    i32 rng = idx.value_or(rand() % 6);

    i32* param_ptr[] = {
        &w.y,
        &w.need,
        &w.shape,
        &w.link_no,
        &w.tear,
        &w.waste
    };

    auto param = param_ptr[rng];
    auto param_pre = *param;

    auto value = 1 + (rand() % 10);

    *param += value;
    w1 = constrain(w);

    *param = param_pre;
    *param -= value;
    w2 = constrain(w);

    return { w1, w2 };
};

i32 match(Eval::Weight w, Eval::Weight w1, Eval::Weight w2, i32 result[6])
{
    std::atomic<i32> count = 0;
    std::atomic<i32> count1 = 0;
    std::atomic<i32> count2 = 0;

    std::atomic<i32> score = 0;
    std::atomic<i32> score1 = 0;
    std::atomic<i32> score2 = 0;

    std::vector<Cell::Pair> queues[100];
    for (i32 i = 0; i < 100; ++i) {
        queues[i] = create_queue();
    }

    std::vector<std::thread> threads;

    std::atomic<i32> progress = 0;

    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] (i32 tid) {
            for (i32 i = 0; i < 25; ++i) {
                auto queue = queues[tid * 25 + i];

                auto sim = simulate(w, queue);
                auto sim1 = simulate(w1, queue);
                auto sim2 = simulate(w2, queue);

                if (sim >= 78000) {
                    count += 1;
                    score += sim;
                }

                if (sim1 >= 78000) {
                    count1 += 1;
                    score1 += sim1;
                }

                if (sim2 >= 78000) {
                    count2 += 1;
                    score2 += sim2;
                }

                progress += 1;
                printf("\rprogress: %d", progress.load());
            }
        }, t);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    printf("\n");

    struct Score {
        i32 count = 0;
        i32 score = 0;

        bool operator < (Score other)  {
            if (this->count != other.count) {
                return this->count < other.count;
            }

            return this->score < other.score;
        };
    };

    Score s = { count.load(), score.load() };
    Score s1 = { count1.load(), score1.load() };
    Score s2 = { count2.load(), score2.load() };

    result[0] = count.load();
    result[1] = score.load();
    result[2] = count1.load();
    result[3] = score1.load();
    result[4] = count2.load();
    result[5] = score2.load();

    if (s1 < s && s2 < s) {
        return 0;
    }

    if (s2 < s && s < s1) {
        return 1;
    }

    if (s1 < s && s < s2) {
        return -1;
    }

    if (s1 < s2) {
        return -1;
    }

    return 1;
};

static void print_w(Eval::Weight w)
{
    #define PRW(p) printf("%s: %d\n", #p, w.p);

    PRW(y);
    PRW(need);
    PRW(shape);
    PRW(link_no);
    PRW(tear);
    PRW(waste);
};

static void run(Eval::Weight w)
{
    i32 unchange = 0;

    i32 id = 0;

    i32 pidx = 0;

    while (true)
    {
        auto randw = Tuner::randomize(w, pidx);

        pidx += 1;
        pidx = pidx % 6;

        auto w1 = randw.first;
        auto w2 = randw.second;

        i32 result[6] = { 0 };

        auto m = Tuner::match(w, w1, w2, result);

        if (m == 1) {
            unchange = 0;
            w = w1;
        }
        else if (m == -1) {
            unchange = 0;
            w = w2;
        }
        else {
            unchange += 1;

            if (unchange >= 10) {
                break;
            }

            continue;
        }

        // Tuner::save(std::to_string(id), w);
        auto save_data = SaveData{
            .w = w,
            .count = 0,
            .score = 0,
        };

        if (m == 1) {
            save_data.count = result[2];
            save_data.score = result[3] / result[2];
        }

        if (m == -1) {
            save_data.count = result[4];
            save_data.score = result[5] / result[4];
        }

        Tuner::save(std::to_string(id), save_data);

        system("cls");
        printf("id: %d\n\n", id);
        printf("w0: %d - %d\n", result[0], result[1] / result[0]);
        printf("w+: %d - %d\n", result[2], result[3] / result[2]);
        printf("w-: %d - %d\n", result[4], result[5] / result[4]);
        printf("\n");
        Tuner::print_w(w);

        id += 1;
    }
};

};