#pragma once

#include "../ai/ai.h"

namespace tuner
{

struct Score
{
    chain::Score chain = { 0, 0 };
    i32 frame = 0;
};

inline Score get_score(beam::eval::Weight w, u32 seed)
{
    auto score = Score();

    auto field = Field();
    auto queue = cell::create_queue(seed);

    for (i32 i = 0; i < 64; ++i) {
        cell::Queue q = {
            queue[(i + 0) % 128],
            queue[(i + 1) % 128]
        };

        auto ai = beam::search_multi(field, q, w);

        if (ai.candidates.empty()) {
            score = Score {
                .chain = 0,
                .frame = 1
            };
            
            break;
        }

        auto mv = ai.candidates.front();

        score.frame += field.get_drop_pair_frame(mv.placement.x, mv.placement.r);

        field.drop_pair(mv.placement.x, mv.placement.r, q[0]);

        auto mask = field.pop();
        auto chain = chain::get_score(mask);

        if (field.get_height(2) > 11) {
            break;
        }

        if (chain.score > score.chain.score) {
            score.chain = chain;
        }

        if (chain.score >= 80000) {
            break;
        }
        
        score.frame += chain.count * 2;
    }

    score.chain.score = i32(score.chain.score / 1000) * 1000;

    return score;
};

};