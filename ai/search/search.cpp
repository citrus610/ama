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
bool Thread::search(Field field, cell::Queue queue, Configs configs, std::optional<i32> trigger, bool stretch)
{
    if (this->thread != nullptr) {
        return false;
    }

    this->clear();

    this->thread = new std::thread([&] (Field f, cell::Queue q, Configs w, std::optional<i32> t, bool s) {
        auto r = Result();

        auto beam_configs = beam::Configs();

        if (t.has_value()) {
            beam_configs.trigger = t.value();
            beam_configs.stretch = s;
        }

        if (q.size() > 2) {
            r.build = beam::search(f, q, w.build, beam_configs);

            if (!r.build.candidates.empty()) {
                std::sort(
                    r.build.candidates.begin(),
                    r.build.candidates.end(),
                    [&] (const beam::Candidate& a, const beam::Candidate& b) {
                        if (beam_configs.stretch) {
                            return a.score > b.score;
                        }

                        bool a_enough = a.score / beam::BRANCH >= beam_configs.trigger;
                        bool b_enough = b.score / beam::BRANCH >= beam_configs.trigger;

                        if (a_enough && b_enough) {
                            return a.score < b.score;
                        }

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
    }, field, queue, configs, trigger, stretch);

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