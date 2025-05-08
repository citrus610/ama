#include "search.h"

namespace search
{

Thread::Thread()
{
    this->thread = nullptr;
    this->results = {};
};

// Starts the search thread
// We search all the configuration weights provided
bool Thread::search(Field field, cell::Queue queue, Configs configs, std::optional<i32> trigger)
{
    if (this->thread != nullptr) {
        return false;
    }

    this->clear();

    this->thread = new std::thread([&] (Field f, cell::Queue q, Configs w) {
        auto r = Result();

        auto beam_configs = beam::Configs();

        if (trigger.has_value()) {
            beam_configs.trigger = trigger.value();
        }

        if (q.size() > 2) {
            r.build = beam::search(f, q, w.build, beam_configs);

            if (!r.build.candidates.empty()) {
                std::sort(
                    r.build.candidates.begin(),
                    r.build.candidates.end(),
                    [] (const beam::Candidate& a, const beam::Candidate& b) {
                        return a.score > b.score;
                    }
                );
            }
        }
        else {
            r.build = beam::search_multi(f, q, w.build, beam_configs);
            r.freestyle = dfs::build::search(f, q, w.freestyle);
            r.fast = dfs::build::search(f, q, w.fast);
            r.ac = dfs::build::search(f, q, w.ac);
        }

        this->results = r;
    }, field, queue, configs);

    return true;
};

std::optional<Result> Thread::get()
{
    if (this->thread == nullptr) {
        return {};
    }

    if (this->thread->joinable()) {
        this->thread->join();
    };

    auto result = this->results;

    this->clear();

    return result;
};

void Thread::clear()
{
    if (this->thread != nullptr) {
        if (this->thread->joinable()) {
            this->thread->join();
        }

        delete this->thread;
    }

    this->thread = nullptr;
    this->results = {};
};

};