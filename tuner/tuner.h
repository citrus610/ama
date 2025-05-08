#pragma once

#include <fstream>
#include "score.h"

namespace tuner
{

struct Save
{
    beam::eval::Weight w;
    std::array<Score, 3> score;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Score, chain.count, chain.score, frame)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Save, w, score)

void save(i32 id, Save s)
{
    std::string name = std::string("data/") + std::to_string(id) + std::string(".json");
    std::ofstream o(name);

    json js;
    to_json(js, s);

    o << std::setw(4) << js << std::endl;
    o.close();
};

Save load(i32 id)
{
    std::string name = std::string("data/") + std::to_string(id) + std::string(".json");
    std::ifstream file;

    file.open(name);

    json js;
    file >> js;
    file.close();

    Save s;
    from_json(js, s);

    return s;
};

beam::eval::Weight constrain(beam::eval::Weight w)
{
    #define CONSTRAIN_POSITIVE(p) w.p = std::max(0, w.p);
    #define CONSTRAIN_NEGATIVE(p) w.p = std::min(0, w.p);
    #define CONSTRAIN_CLAMP(p, m1, m2) w.p = std::clamp(w.p, m1, m2);

    CONSTRAIN_POSITIVE(chain)
    CONSTRAIN_POSITIVE(y)
    CONSTRAIN_NEGATIVE(key)
    CONSTRAIN_POSITIVE(chi)
    CONSTRAIN_NEGATIVE(shape)
    CONSTRAIN_NEGATIVE(well)
    CONSTRAIN_NEGATIVE(bump)
    CONSTRAIN_POSITIVE(form)
    CONSTRAIN_CLAMP(link_2, 50, 200)
    CONSTRAIN_CLAMP(link_3, 150, 500)
    // CONSTRAIN_NEGATIVE(waste_14)
    // CONSTRAIN_POSITIVE(side)
    // CONSTRAIN_POSITIVE(nuisance)
    CONSTRAIN_NEGATIVE(tear)
    CONSTRAIN_NEGATIVE(waste)

    return w;
};

void move_toward(beam::eval::Weight& w, beam::eval::Weight target, i32 id)
{
    #define MOVE_TOWARD(p, r) w.p += i32(std::round(double(target.p - w.p) * r));

    MOVE_TOWARD(chain, 0.1)
    MOVE_TOWARD(y, 0.1)
    MOVE_TOWARD(key, 0.1)
    MOVE_TOWARD(chi, 0.1)
    MOVE_TOWARD(shape, 0.1)
    MOVE_TOWARD(well, 0.1)
    MOVE_TOWARD(bump, 0.1)
    MOVE_TOWARD(form, 0.1)
    MOVE_TOWARD(link_2, 0.1)
    MOVE_TOWARD(link_3, 0.1)
    MOVE_TOWARD(waste_14, 0.1)
    MOVE_TOWARD(side, 0.1)
    MOVE_TOWARD(nuisance, 0.1)
    MOVE_TOWARD(tear, 0.1)
    MOVE_TOWARD(waste, 0.1)
};

std::pair<beam::eval::Weight, beam::eval::Weight> randomize(beam::eval::Weight w, i32 id)
{
    auto w1 = w;
    auto w2 = w;
    auto w_pre = w;

    std::pair<i32*, i32> param[] = {
        // { &w.chain, 50 },
        { &w.y, 10 },
        { &w.key, 10 },
        { &w.chi, 10 },
        { &w.shape, 10 },
        { &w.well, 10 },
        { &w.bump, 10 },
        // { &w.form, 20 },
        { &w.link_2, 10 },
        { &w.link_3, 10 },
        // { &w.waste_14, 20 },
        // { &w.side, 20 },
        // { &w.nuisance, 20 },
        { &w.tear, 10 },
        { &w.waste, 10 }
    };

    i32 param_delta[_countof(param)] = { 0 };

    for (size_t i = 0; i < _countof(param); ++i) {
        i32 delta = param[i].second;

        i32 sign = (rand() % 2) * 2 - 1;
        i32 value = delta;

        param_delta[i] = value * sign;
    }

    for (size_t i = 0; i < _countof(param); ++i) {
        *param[i].first += param_delta[i];
    }

    w1 = constrain(w);

    w = w_pre;

    for (size_t i = 0; i < _countof(param); ++i) {
        *param[i].first -= param_delta[i];
    }

    w2 = constrain(w);

    return { w1, w2 };
};

i32 match(u32 seed, beam::eval::Weight w1, beam::eval::Weight w2, Score& r1, Score& r2)
{
    auto cmp_result = [] (Score s1, Score s2) -> bool {
        bool over_1 = s1.chain.score >= 95000;
        bool over_2 = s2.chain.score >= 95000;

        if (over_1 != over_2) {
            return s1.chain.score < s2.chain.score;
        }

        double eff_1 = double(s1.chain.score) / double(s1.frame);
        double eff_2 = double(s2.chain.score) / double(s2.frame);

        return eff_1 < eff_2;
    };

    r1 = get_score(w1, seed);
    r2 = get_score(w2, seed);

    printf("\n");

    system("cls");

    printf("w+: chain - %d | score - %d | frame - %d\n", r1.chain.count, r1.chain.score, r1.frame);
    printf("w-: chain - %d | score - %d | frame - %d\n", r2.chain.count, r2.chain.score, r2.frame);

    if (cmp_result(r2, r1)) {
        return 1;
    }

    if (cmp_result(r1, r2)) {
        return -1;
    }

    return 0;
};

inline void run(beam::eval::Weight w, i32 id_init = 0)
{
    system("cls");

    i32 id = id_init;

    while (true)
    {
        auto [w1, w2] = tuner::randomize(w, id);

        Score s0, s1, s2;

        u32 seed = rand() & 0xFFFF;

        i32 m = tuner::match(seed, w1, w2, s1, s2);

        if (m == 1) {
            tuner::move_toward(w, w1, id);
        }
        else if (m == -1) {
            tuner::move_toward(w, w2, id);
        }
        else {
            continue;
        }

        s0 = get_score(w, seed);

        printf("w0: chain - %d | score - %d | frame - %d\n", s0.chain.count, s0.chain.score, s0.frame);

        auto save_data = Save{
            .w = w,
            .score = { s0, s1, s2 }
        };

        tuner::save(id, save_data);

        printf("id: %d\n\n", id);

        id += 1;
    }
};

};