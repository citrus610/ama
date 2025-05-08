#include "../core/core.h"
#include "../ai/ai.h"

void load_json(beam::eval::Weight& h)
{
    std::ifstream file;
    file.open("config.json");
    json js;
    file >> js;
    file.close();
    from_json(js, h);
};

void save_json()
{
    std::ifstream f("config.json");
    if (f.good()) {
        return;
    };
    f.close();

    std::ofstream o("config.json");
    json js;
    to_json(js, beam::eval::Weight());
    o << std::setw(4) << js << std::endl;
    o.close();
};

inline chain::Score get_score(beam::eval::Weight w, u32 seed)
{
    auto score = chain::Score();

    auto field = Field();
    auto queue = cell::create_queue(seed);

    for (i32 i = 0; i < 50; ++i) {
        cell::Queue q = {
            queue[(i + 0) % 128],
            queue[(i + 1) % 128]
        };

        auto ai = beam::search_multi(field, q, w);

        if (ai.candidates.empty()) {
            break;
        }

        auto mv = ai.candidates.front();

        field.drop_pair(mv.placement.x, mv.placement.r, q[0]);

        auto mask = field.pop();
        auto chain = chain::get_score(mask);

        if (field.get_height(2) > 11) {
            break;
        }

        if (chain.score > score.score) {
            score = chain;
        }

        if (chain.score >= 80000) {
            break;
        }
    }

    return score;
};

int main()
{
    beam::eval::Weight w;
    save_json();
    load_json(w);

    srand(uint32_t(time(NULL)));

    const i32 POPULATION_SIZE = 200;

    u32 seeds[POPULATION_SIZE];

    for (i32 i = 0; i < POPULATION_SIZE; ++i) {
        seeds[i] = rand() & 0xFFFF;
    }

    std::atomic<i32> progress = 0;
    std::vector<std::thread> threads;

    std::atomic<i32> map_count[20] = { 0 };
    std::atomic<i32> map_score[POPULATION_SIZE] = { 0 };

    struct FailedSeed
    {
        u32 seed = ~0;
        i32 score = 0;
    };

    std::vector<FailedSeed> failed_seed = {};

    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] (i32 tid) {
            for (i32 i = 0; i < POPULATION_SIZE / 4; ++i) {
                auto seed = seeds[tid * POPULATION_SIZE / 4 + i];

                auto score = get_score(w, seed);

                map_count[score.count] += 1;
                map_score[tid * POPULATION_SIZE / 4 + i] = score.score;

                if (score.score < 90000) {
                    failed_seed.push_back(FailedSeed { .seed = seeds[tid * POPULATION_SIZE / 4 + i], .score = score.score });
                }

                progress += 1;
                printf("\rprogress: %d", progress.load());
            }
        }, t);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::string out_str;

    for (i32 i = 0; i < 20; ++i) {
        out_str += std::to_string(map_count[i].load()) + "\n";
    }

    out_str += "\n";

    for (i32 i = 0; i < POPULATION_SIZE; ++i) {
        out_str += std::to_string(map_score[i].load()) + "\n";
    }

    out_str += "\n";

    std::sort(
        failed_seed.begin(),
        failed_seed.end(),
        [&] (FailedSeed a, FailedSeed b) {
            return a.score < b.score;
        }
    );

    for (i32 i = 0; i < failed_seed.size(); ++i) {
        out_str += std::to_string(failed_seed[i].seed) + " " + std::to_string(failed_seed[i].score) + "\n";
    }

    std::ofstream o("out.txt");
    o << out_str << std::endl;
    o.close();

    return 0;
};