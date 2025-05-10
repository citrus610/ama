#include "layer.h"

namespace beam
{

Layer::Layer(size_t width)
{
    this->width = width;
    this->data.reserve(width);
    this->table.resize();
};

void Layer::clear()
{
    this->data.clear();
    this->table.update();
};

// Add a node into the layer
void Layer::add(node::Data& node)
{
    // If the layer's size is smaller than the beam's width, we simply push the node
    // We then check if the layer's size is big enough, then we turn the data into a binary heap
    if (this->data.size() < this->width) {
        this->data.push_back(node);

        if (this->data.size() == this->width) {
            std::make_heap(
                this->data.begin(),
                this->data.end(),
                [&] (const node::Data& a, const node::Data& b) { return b < a; }
            );
        }

        return;
    }
    
    // When the layer's size is big enough, we compare the node against the weakest node in the layer
    // We don't push the new node if its eval is smaller than the weakest node's eval
    // Push by pop heap and push heap
    if (this->data.front() < node) {
        std::pop_heap(
            this->data.begin(),
            this->data.end(),
            [&] (const node::Data& a, const node::Data& b) { return b < a; }
        );

        this->data.back() = node;

        std::push_heap(
            this->data.begin(),
            this->data.end(),
            [&] (const node::Data& a, const node::Data& b) { return b < a; }
        );
    }
};

// Sorts the nodes in the layer
void Layer::sort()
{
    if (this->data.size() < this->width) {
        std::sort(
            this->data.begin(),
            this->data.end(),
            [&] (node::Data& a, node::Data& b) { return b < a; }
        );

        return;
    }

    std::sort_heap(
        this->data.begin(),
        this->data.end(),
        [&] (node::Data& a, node::Data& b) { return b < a; }
    );
};

};