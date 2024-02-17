#pragma once

#include "../ai/ai.h"

static std::pair<i32, i32> simulate(Eval::Weight w, std::vector<Cell::Pair> queue)
{
    Field field = Field();

    i32 score = 0;
    i32 frame = 0;

    for (int i = 0; i < 50; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        auto airesult = AI::think_1p(field, tqueue, w, false);

        frame += field.get_drop_pair_frame(airesult.placement.x, airesult.placement.r);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            return { 0, 0 };
        }

        if (chain.score > 0) {
            score = std::max(score, chain.score);

            if (chain.score >= 78000) {
                break;
            }
        }

        frame += chain.count * 2;
    }

    return { score, frame };
};