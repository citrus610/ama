#pragma once

#include "move.h"

namespace Path
{

typedef avec<Move::Type, 32> Queue;

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
};

static void print(Queue& queue)
{
    for (i32 i = 0; i < queue.get_size(); ++i) {
        if (queue[i] == Move::Type::NONE) {
            continue;
        }
        std::cout << Move::to_str(queue[i]) << " ";
    }
};

static Queue get_queue_normalized(Queue& queue)
{
    Queue result = Queue();

    for (i32 i = 0; i < queue.get_size(); ++i) {
        if (queue[i] != Move::Type::M180) {
            result.add(queue[i]);
            continue;
        }

        Move::Type m180_rotate_type = Move::Type::CW;
        for (i32 k = i; k < queue.get_size(); ++k) {
            if (queue[k] == Move::Type::CW) {
                m180_rotate_type = Move::Type::CCW;
                break;
            }
            if (queue[k] == Move::Type::CCW) {
                m180_rotate_type = Move::Type::CW;
                break;
            }
        }

        result.add(m180_rotate_type);
        result.add(Move::Type::NONE);
        result.add(m180_rotate_type);
        result.add(Move::Type::LEFT);
        result.add(Move::Type::NONE);
        result.add(Move::Type::LEFT);
        result.add(Move::Type::NONE);
        result.add(Move::Type::LEFT);
        result.add(Move::Type::NONE);
        result.add(Move::Type::LEFT);
        result.add(Move::Type::NONE);

        // if (i + 1 < queue.get_size() && queue[i + 1] == m180_rotate_type) {
        //     result.add(Move::Type::NONE);
        // }
    }

    return result;
};

};