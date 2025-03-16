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
    beam::Result build;
    dfs::build::Result freestyle;
    dfs::build::Result fast;
    dfs::build::Result ac;
};

class Thread
{
private:
    std::thread* thread;
    std::optional<Result> results;
public:
    Thread();
public:
    bool search(Field field, cell::Queue queue, Configs configs);
    std::optional<Result> get();
    void clear();
};

};