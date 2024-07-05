#pragma once

#include "move.h"

namespace Path
{

enum class Input
{
    LEFT,
    RIGHT,
    CW,
    CCW,
    M180,
    DROP,
    NONE,
    WAIT
};

typedef std::vector<Input> Queue;

class Position
{
public:
    i8 x = 0;
    i8 y = 0;
    Direction::Type r = Direction::Type::UP;
public:
    bool move_right(Field& field, u8 height[6]);
    bool move_left(Field& field, u8 height[6]);
    bool move_cw(Field& field, u8 height[6]);
    bool move_ccw(Field& field, u8 height[6]);
    bool move_180(Field& field, u8 height[6]);
public:
    bool is_colliding(u8 height[6]);
public:
    void normalize();
};

class PositionMap
{
public:
    Queue data[6][5][4];
public:
    PositionMap();
public:
    void clear();
public:
    Queue get(i8 x, i8 y, Direction::Type direction);
    void set(i8 x, i8 y, Direction::Type direction, Queue value);
};

class PlacementMap
{
public:
    Queue data[6][4];
public:
    PlacementMap();
public:
    void clear();
public:
    Queue get(u8 x, Direction::Type direction);
    void set(u8 x, Direction::Type direction, Queue value);
};

class Finder
{
public:
    struct Node
    {
        Position position;
        Queue path;
    };
public:
    static Queue find(Field& field, Move::Placement placement, Cell::Pair pair);
    static void expand(Field& field, u8 height[6], Node& node, std::vector<Node>& queue, PositionMap& queue_map);
    static void lock(Node& node, PlacementMap& locks_map, bool equal_pair);
    static bool above_stack_move(Field& field, Move::Placement placement);
    static Queue get_queue_convert_m180(Queue& queue);
};

static void print(Queue queue)
{
    for (auto mv : queue) {
        switch (mv)
        {
        case Input::LEFT:
            printf("L ");
            break;
        case Input::RIGHT:
            printf("R ");
            break;
        case Input::CW:
            printf("CW ");
            break;
        case Input::CCW:
            printf("CCW ");
            break;
        case Input::M180:
            printf("180 ");
            break;
        case Input::DROP:
            printf("D ");
            break;
        case Input::NONE:
            printf("_ ");
            break;
        default:
            break;
        }
    }

    printf("\n");
};

};