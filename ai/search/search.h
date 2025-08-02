#pragma once

#include "beam/beam.h"
#include "dfs/build.h"
#include "dfs/attack.h"

namespace search
{

enum Type
{
    BUILD,
    FREESTYLE,
    FAST,
    AC
};

struct Configs
{
    beam::eval::Weight build;
    dfs::eval::Weight freestyle;
    dfs::eval::Weight fast;
    dfs::eval::Weight ac;
};

struct Result
{
    beam::Result build = beam::Result();
    dfs::build::Result freestyle = dfs::build::Result();
    dfs::build::Result fast = dfs::build::Result();
    dfs::build::Result ac = dfs::build::Result();
};

// Containter for searching multiple sets of weights during moving pair
class Thread
{
private:
    std::thread* thread;
    std::optional<Result> results;
public:
    Thread();
public:
    bool search(Field field, cell::Queue queue, Configs configs, std::optional<i32> trigger = {}, bool stretch = true);
    std::optional<Result> get();
    void clear();
};

};