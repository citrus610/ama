#pragma once

#include "table.h"
#include "eval.h"

namespace beam
{

class Layer
{
public:
    std::vector<node::Data> data;
    Table table;
    size_t width;
public:
    Layer(size_t width);
public:
    void clear();
    void add(node::Data& node, const eval::Weight& w);
    void sort();
};

};