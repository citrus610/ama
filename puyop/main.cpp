#include <iostream>
#include "encode.h"

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

int main()
{
    using namespace std;

    srand(uint32_t(time(NULL)));

    Eval::Weight heuristic;
    save_json_heuristic();
    load_json_heuristic(heuristic);

    auto queue = Cell::create_queue(rand() & 0xFFFF);

    Field field;

    vector<Move::Placement> placements;

    i32 time = 0;
    i32 score = 0;

    for (i32 i = 0; i < 100; ++i) {
        if (field.get_height(2) > 11) {
            break;
        }

        vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % 128]);
        tqueue.push_back(queue[(i + 1) % 128]);
        tqueue.push_back(queue[(i + 2) % 128]);

        auto time_start = std::chrono::high_resolution_clock::now();
        auto ai_result = AI::think_1p(field, tqueue, heuristic);
        auto time_stop = std::chrono::high_resolution_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start).count();
        time += dt;

        Cell::Pair pair = { tqueue[0].first, tqueue[0].second };

        field.drop_pair(ai_result.placement.x, ai_result.placement.r, pair);
        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        placements.push_back(ai_result.placement);

        i32 plan_score = 0;
        auto plan = ai_result.plan;
        if (plan.has_value()) {
            auto pop_mask = plan.value().pop();

            plan_score = Chain::get_score(pop_mask).score;
        }

        if (ai_result.plan.has_value()) {
            printf("eval: %d - ets: %d", ai_result.eval, plan_score);
        }
        else {
            printf("chain: %d", ai_result.eval);
        }

        printf(" - %d ms\n", dt);

        if (chain.score >= 78000) {
            score = chain.score;
            break;
        }
    }

    cout << Encode::get_encoded_URL(Field(), queue, placements) << "\n";
    cout << "time: " << std::to_string(double(time) / double(placements.size())) << " ms\n";
    cout << "score: " << score << "\n";

    return 0;
};