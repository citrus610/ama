#include <iostream>
#include "encode.h"

std::vector<Cell::Pair> create_queue()
{
    using namespace std;

    vector<Cell::Type> bag3;
    bag3.reserve(256);

    while (true)
    {
        bool full = false;
        for (u8 p = 0; p < Cell::COUNT - 2; ++p) {
            bag3.push_back(Cell::Type(p));
            if (bag3.size() >= 256) {
                full = true;
                break;
            }
        }
        if (full) {
            break;
        }
    }

    for (int i = 255; i >= 0; --i) {
        int k = rand() % 256;
        Cell::Type value = bag3[i];
        bag3[i] = bag3[k];
        bag3[k] = value;
    }

    vector<Cell::Type> bag;
    bag.reserve(256);

    for (int i = 0; i < 64; ++i) {
        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            bag.push_back(Cell::Type(p));
        }
    }

    for (int i = 255; i >= 0; --i) {
        int k = rand() % 256;
        Cell::Type value = bag[i];
        bag[i] = bag[k];
        bag[k] = value;
    }

    bag[0] = bag3[0];
    bag[1] = bag3[1];
    bag[2] = bag3[2];
    bag[3] = bag3[3];

    vector<Cell::Pair> queue;
    queue.reserve(128);

    for (int i = 0; i < 128; ++i) {
        queue.push_back({ bag[i * 2], bag[i * 2 + 1] });
    }

    return queue;
};

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

    vector<Cell::Pair> queue = create_queue();

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
        AI::Result ai_result = AI::think_1p(field, tqueue, heuristic);
        auto time_stop = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start).count();

        Cell::Pair pair = { tqueue[0].first, tqueue[0].second };

        field.drop_pair(ai_result.placement.x, ai_result.placement.r, pair);
        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        placements.push_back(ai_result.placement);

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