#pragma once

#include "../ai/ai.h"

static std::vector<Cell::Pair> create_queue()
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

static i32 simulate(Eval::Weight w, std::vector<Cell::Pair> queue)
{
    Field field = Field();

    i32 score = 0;

    for (int i = 0; i < 50; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        AI::Result airesult = AI::RESULT_DEFAULT;

        auto search = Search::search(field, tqueue);

        if (!search.candidates.empty()) {
            AI::get_candidate_eval(search, w);
            airesult = AI::build(search, field, 78000, false);
        }

        auto drop_frame = field.get_drop_pair_frame(airesult.placement.x, airesult.placement.r);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            return 0;
        }

        if (chain.score > 0) {
            score = std::max(score, chain.score);
        }
    }

    return score;
};