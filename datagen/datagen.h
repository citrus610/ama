#pragma once

#include "format.h"

namespace datagen
{

constexpr usize OPENING = 8;
constexpr usize GOAL = 50000000;

constexpr auto WEIGHT = beam::eval::Weight {
    .chain = 1000,
    .y = 289,
    .key = -200,
    .chi = 200,
    .shape = -100,
    .well = -100,
    .bump = -100,
    .form = 200,
    .link_2 = 150,
    .link_3 = 250,
    .waste_14 = -50,
    .side = 0,
    .nuisance = -250,
    .tear = -250,
    .waste = -250
};

class Rng
{
public:
    u64 seed;
public:
    Rng(u64 id = 0) {
        seed = time(NULL) + id;
    };
public:
    u64 get() {
        this->seed ^= this->seed >> 12;
        this->seed ^= this->seed << 25;
        this->seed ^= this->seed >> 27;

        return this->seed * 2685821657736338717Ull;
    };
};

inline i32 get_score(Field field)
{
    auto beam = beam::search_multi(field, {}, WEIGHT, { 250ULL, 8ULL, 100000ULL, true });

    if (beam.candidates.empty()) {
        return 0;
    }

    return beam.candidates.front().score / beam::BRANCH;
};

inline std::vector<format::Data> get_simulation(Rng& rng)
{
    std::vector<format::Data> result;

    auto field = Field();
    auto queue = cell::create_queue(rng.get() & 0xFFFF);

    for (usize i = 0; i < OPENING; ++i) {
        auto pair = queue[i];
        auto moves = move::generate(field, pair.first == pair.second);

        if (moves.get_size() == 0) {
            return get_simulation(rng);
        }

        auto move = moves[rng.get() % moves.get_size()];

        field.drop_pair(move.x, move.r, pair);
        field.pop();

        if (field.get_height(2) >= 12) {
            return get_simulation(rng);
        }
    }

    for (i32 i = 0; i < 50; ++i) {
        std::vector<cell::Pair> q;
        q.push_back(queue[(i + 0) % 128]);
        q.push_back(queue[(i + 1) % 128]);

        auto ai_result = beam::search_multi(field, q, WEIGHT);

        if (ai_result.candidates.empty()) {
            break;
        }

        result.push_back(format::get_from_field(field, get_score(field)));

        auto mv = ai_result.candidates[0];

        field.drop_pair(mv.placement.x, mv.placement.r, q.front());

        auto mask = field.pop();
        auto chain = chain::get_score(mask);

        if (field.get_height(2) > 11 || chain.score >= 50000) {
            break;
        }
    }

    return result;
};

inline void run()
{
    // Creates rng
    auto rng = Rng(0);
    
    // Loops
    usize queue = 0;
    usize entry = 0;
    std::vector<format::Data> buffer;

    while (true) {
        auto pos = get_simulation(rng);

        buffer.insert(buffer.end(), pos.begin(), pos.end());

        queue += 1;
        entry += pos.size();

        if (buffer.size() >= 100) {
            auto out = std::ofstream("data.bin", std::ios::out | std::ios::binary | std::ios::app);

            for (auto p : buffer) {
                out.write(reinterpret_cast<const char*>(&p), format::SIZE);
            }

            buffer.clear();
            out.close();

            std::cout << "\rprogress: " << (f64(entry) / f64(GOAL) * 100.0) << " | entries: " << entry << "/" << GOAL << "                        ";
        }

        if (entry >= GOAL) {
            break;
        }
    }
};

};