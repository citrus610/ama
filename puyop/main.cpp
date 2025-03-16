#include <iostream>
#include "encode.h"

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

int main(int argc, char** argv)
{
    using namespace std;

    srand(uint32_t(time(NULL)));

    beam::eval::Weight w;
    save_json();
    load_json(w);

    u32 seed = rand() & 0xFFFF;
    seed = rand() & 0xFFFF;

    if (argc == 2) {
        seed = std::atoi(argv[1]);
    }

    printf("seed: %d\n", seed);

    auto queue = cell::create_queue(seed);

    Field field;

    vector<move::Placement> placements;

    i32 time = 0;
    i32 score = 0;

    for (i32 i = 0; i < 100; ++i) {
        if (field.get_height(2) > 11) {
            break;
        }

        vector<cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % 128]);
        tqueue.push_back(queue[(i + 1) % 128]);

        auto time_start = std::chrono::high_resolution_clock::now();
        auto ai_result = beam::search_multi(field, tqueue, w);
        auto time_stop = std::chrono::high_resolution_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start).count();
        time += dt;

        if (ai_result.candidates.empty()) {
            break;
        }

        auto mv = ai_result.candidates[0];

        field.drop_pair(mv.placement.x, mv.placement.r, tqueue[0]);
        auto mask = field.pop();
        auto chain = chain::get_score(mask);

        placements.push_back(mv.placement);

        i32 plan_score = mv.score;

        printf("ets: %d", plan_score / beam::BRANCH);
        printf(" - %d ms\n", dt);

        if (chain.score >= 78000) {
            score = chain.score;
            break;
        }
    }

    cout << encode::get_encoded_URL(Field(), queue, placements) << "\n";
    cout << "time: " << std::to_string(double(time) / double(placements.size())) << " ms\n";
    cout << "score: " << score << "\n";

    return 0;
};