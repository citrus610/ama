#include <iostream>
#include "encode.h"

std::vector<Cell::Pair> create_queue()
{
    using namespace std;

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

    for (i32 i = 0; i < 100; ++i) {
        if (field.get_height(2) > 11) {
            break;
        }

        vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % 128]);
        tqueue.push_back(queue[(i + 1) % 128]);
        tqueue.push_back(queue[(i + 2) % 128]);

        auto time_start = std::chrono::high_resolution_clock::now();
        AI::Result ai_result = AI::think_1p(field, tqueue, heuristic, 80000);
        auto time_stop = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start).count();

        Cell::Pair pair = { tqueue[0].first, tqueue[0].second };

        field.drop_pair(ai_result.placement.x, ai_result.placement.r, pair);
        field.pop();

        placements.push_back(ai_result.placement);
    }

    cout << Encode::get_encoded_URL(Field(), queue, placements) << "\n";
    cout << "time: " << std::to_string(double(time) / 100.0) << "\n";

    return 0;
};