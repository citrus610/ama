#include "path.h"

namespace path
{

bool Position::move_right(Field& field, u8 height[6])
{
    if (!field.is_colliding_pair(this->x + 1, this->y, this->r, height)) {
        this->x += 1;
        return true;
    }

    return false;
};

bool Position::move_left(Field& field, u8 height[6])
{
    if (!field.is_colliding_pair(this->x - 1, this->y, this->r, height)) {
        this->x -= 1;
        return true;
    }

    return false;
};

bool Position::move_cw(Field& field, u8 height[6])
{
    direction::Type new_direction = direction::Type((static_cast<u8>(this->r) + 1) & 0b11);

    if (!field.is_colliding_pair(this->x, this->y, new_direction, height)) {
        this->r = new_direction;
        return true;
    }

    if (!field.is_colliding_pair(this->x - direction::get_offset_x(new_direction), this->y - direction::get_offset_y(new_direction), new_direction, height)) {
        this->x -= direction::get_offset_x(new_direction);
        this->y -= direction::get_offset_y(new_direction);
        this->r = new_direction;
        return true;
    }

    return false;
};

bool Position::move_ccw(Field& field, u8 height[6])
{
    direction::Type new_direction = direction::Type((static_cast<u8>(this->r) + 3) & 0b11);
    
    if (!field.is_colliding_pair(this->x, this->y, new_direction, height)) {
        this->r = new_direction;
        return true;
    }

    if (!field.is_colliding_pair(this->x - direction::get_offset_x(new_direction), this->y - direction::get_offset_y(new_direction), new_direction, height)) {
        this->x -= direction::get_offset_x(new_direction);
        this->y -= direction::get_offset_y(new_direction);
        this->r = new_direction;
        return true;
    }
    
    return false;
};

bool Position::move_180(Field& field, u8 height[6])
{
    if (this->r == direction::Type::RIGHT ||
        this->r == direction::Type::LEFT ||
        !field.is_occupied(this->x - 1, this->y, height) || 
        !field.is_occupied(this->x + 1, this->y, height)) {
        return false;
    }

    i8 new_direction = (static_cast<i8>(this->r) + 2) & 0b11;
    this->y += new_direction - 1;
    this->r = direction::Type(new_direction);

    return true;
};

// Returns pairs orientation to their canonical forms
void Position::normalize()
{
    switch (this->r)
    {
    case direction::Type::LEFT:
        --this->x;
        this->r = direction::Type::RIGHT;
        break;
    case direction::Type::DOWN:
        --this->y;
        this->r = direction::Type::UP;
        break;
    default:
        break;
    }
};

// Returns pairs orientation to their non-canonical forms
void Position::denormalize()
{
    switch (this->r)
    {
    case direction::Type::RIGHT:
        ++this->x;
        this->r = direction::Type::LEFT;
        break;
    case direction::Type::UP:
        ++this->y;
        this->r = direction::Type::DOWN;
        break;
    default:
        break;
    }
};

PositionMap::PositionMap()
{
    this->clear();
};

void PositionMap::clear()
{
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 5; ++y) {
            for (int r = 0; r < 4; ++r) {
                this->data[x][y][r] = {};
            }
        }
    }
};

Queue PositionMap::get(i8 x, i8 y, direction::Type direction)
{
    assert(x >= 0 && x <= 5);
    assert(y > 10 && y < 16);
    return this->data[x][y - 11][static_cast<u8>(direction)];
};

void PositionMap::set(i8 x, i8 y, direction::Type direction, Queue value)
{
    assert(x >= 0 && x <= 5);
    assert(y > 10 && y < 16);
    this->data[x][y - 11][static_cast<u8>(direction)] = value;
};

PlacementMap::PlacementMap()
{
    this->clear();
};

void PlacementMap::clear()
{
    for (int x = 0; x < 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            this->data[x][r] = {};
        }
    }
};

Queue PlacementMap::get(u8 x, direction::Type direction)
{
    assert(x >= 0 && x <= 5);
    assert(int(direction) >= 0 && int(direction) <= 4);
    return this->data[x][static_cast<uint8_t>(direction)];
};

void PlacementMap::set(u8 x, direction::Type direction, Queue value)
{
    assert(x >= 0 && x <= 5);
    assert(int(direction) >= 0 && int(direction) <= 4);
    this->data[x][static_cast<uint8_t>(direction)] = value;
};

// Finds path to place a pair normally
Queue Finder::find(Field& field, move::Placement placement, cell::Pair pair)
{
    // Checks for simple drop
    if (placement.x == 2 && placement.r == direction::Type::UP) {
        Queue drop;
        drop.push_back(Input::DROP);
        return drop;
    }

    // Generates all possible placements with their shortest paths
    auto locks = Finder::generate_placements(field, placement, pair);

    auto position = Position(i8(placement.x), 0, placement.r);
    Queue result = {};

    // Checks if pair is same-color pair
    if (pair.first == pair.second) {
        auto position_normalize = position;
        auto position_denormalize = position;

        position_normalize.normalize();
        position_denormalize.denormalize();

        auto queue_normalize = locks.get(position_normalize.x, position_normalize.r);
        auto queue_denormalize = locks.get(position_denormalize.x, position_denormalize.r);

        // Compares the paths of canonical placement and non-canonical placement
        result = std::max(
            queue_normalize,
            queue_denormalize,
            [&] (const Queue& a, const Queue& b) {
                bool a_valid = !a.empty();
                bool b_valid = !b.empty();

                if (a_valid != b_valid) {
                    return a_valid < b_valid;
                }

                return a.size() > b.size();
            }
        );
    }
    else {
        // Gets the path normally from the locks map
        result = locks.get(position.x, position.r);
    }

    // Adds drop at the end
    result.push_back(Input::DROP);

    // Checks for move 180 and returns the result
    return Finder::get_queue_convert_m180(result);
};

// Generates all possible placements with their respective shortest paths
PlacementMap Finder::generate_placements(Field& field, move::Placement placement, cell::Pair pair)
{
    bool equal_pair = pair.first == pair.second;

    u8 height[6];
    field.get_heights(height);

    std::vector<Finder::Node> queue;
    PositionMap queue_map = PositionMap();
    PlacementMap locks_map = PlacementMap();

    queue.push_back(Finder::Node {
        .position = Position(2, 11, direction::Type::UP), 
        .path = {}
    });

    queue_map.set(2, 11, direction::Type::UP, {});

    while (!queue.empty())
    {
        Finder::Node node = queue.back();
        queue.pop_back();

        if (queue_map.get(node.position.x, node.position.y, node.position.r).size() != node.path.size()) {
            continue;
        }

        Finder::lock(node, locks_map, equal_pair);
        Finder::expand(field, height, node, queue, queue_map);
    }

    return locks_map;
};

// Expands node by moving right, left, cw, ccw, 180
void Finder::expand(Field& field, u8 height[6], Finder::Node& node, std::vector<Finder::Node>& queue, PositionMap& queue_map)
{
    // Moves right
    Finder::Node right = node;

    if (right.position.move_right(field, height)) {
        // Adds a buffer NONE move for hyper-tapping
        if (!right.path.empty() && right.path.back() == Input::RIGHT) {
            right.path.push_back(Input::NONE);
        }

        // Adds move
        right.path.push_back(Input::RIGHT);

        // Checks map if node had been checked
        if (queue_map.get(right.position.x, right.position.y, right.position.r).size() == 0 ||
            queue_map.get(right.position.x, right.position.y, right.position.r).size() >= right.path.size()) {
            queue.push_back(right);
            queue_map.set(right.position.x, right.position.y, right.position.r, right.path);
        }
    }

    // Moves left
    Finder::Node left = node;

    if (left.position.move_left(field, height)) {
        // Adds a buffer NONE move for hyper-tapping
        if (!left.path.empty() && left.path.back() == Input::LEFT) { 
            left.path.push_back(Input::NONE); 
        } 

        // Adds move
        left.path.push_back(Input::LEFT);

        // Checks map if node had been checked
        if (queue_map.get(left.position.x, left.position.y, left.position.r).size() == 0 || 
            queue_map.get(left.position.x, left.position.y, left.position.r).size() >= left.path.size()) { 
            queue.push_back(left);
            queue_map.set(left.position.x, left.position.y, left.position.r, left.path);
        }
    }

    // Moves cw
    Finder::Node cw = node;

    if (cw.position.move_cw(field, height)) {
        // Adds a buffer NONE move for hyper-tapping
        if (!cw.path.empty() && cw.path.back() == Input::CW) {
            cw.path.push_back(Input::NONE); 
        }

        // Adds move
        cw.path.push_back(Input::CW);

        // Checks map if node had been checked
        if (queue_map.get(cw.position.x, cw.position.y, cw.position.r).size() == 0 || 
            queue_map.get(cw.position.x, cw.position.y, cw.position.r).size() >= cw.path.size()) {
            queue.push_back(cw);
            queue_map.set(cw.position.x, cw.position.y, cw.position.r, cw.path);
        }
    }

    // Moves ccw
    Finder::Node ccw = node;

    if (ccw.position.move_ccw(field, height)) {
        // Adds a buffer NONE move for hyper-tapping
        if (!ccw.path.empty() && ccw.path.back() == Input::CCW) { 
            ccw.path.push_back(Input::NONE); 
        }

        // Adds move
        ccw.path.push_back(Input::CCW);

        // Checks map if node had been checked
        if (queue_map.get(ccw.position.x, ccw.position.y, ccw.position.r).size() == 0 || 
            queue_map.get(ccw.position.x, ccw.position.y, ccw.position.r).size() >= ccw.path.size()) {
            queue.push_back(ccw);
            queue_map.set(ccw.position.x, ccw.position.y, ccw.position.r, ccw.path);
        }
    }

    // Moves 180
    Finder::Node m180 = node;

    if (m180.position.move_180(field, height)) {
        // Adds a buffer NONE move for hyper-tapping
        if (!m180.path.empty() && m180.path.back() == Input::M180) { 
            m180.path.push_back(Input::NONE); 
        }

        // Adds move
        m180.path.push_back(Input::M180);

        // Checks map if node had been checked
        if (queue_map.get(m180.position.x, m180.position.y, m180.position.r).size() == 0 || 
            queue_map.get(m180.position.x, m180.position.y, m180.position.r).size() >= m180.path.size()) {
            queue.push_back(m180);
            queue_map.set(m180.position.x, m180.position.y, m180.position.r, m180.path);
        }
    }
};

// Locks the position and updates the locks map
void Finder::lock(Finder::Node& node, PlacementMap& locks_map, bool equal_pair)
{
    if (locks_map.get(node.position.x, node.position.r).size() == 0 ||
        locks_map.get(node.position.x, node.position.r).size() > node.path.size()) {
        locks_map.set(node.position.x, node.position.r, node.path);
    }
};

// Checks if this placement is an above-stack placement or not
// Above-stack placement is defined as placement that require maneuvering on the 13th row
bool Finder::above_stack_move(Field& field, move::Placement placement, u8 stack)
{
    u8 heights[6];
    field.get_heights(heights);

    // If we're placing with rotation DOWN, the stack height should be lower
    if (placement.r == direction::Type::DOWN) {
        if (stack >= 1) {
            stack -= 1;
        }
        else {
            stack = 0;
        }
    }

    if (placement.x > 2) {
        for (int x = 2; x <= placement.x; ++x) {
            if (heights[x] >= stack) {
                return true;
            }
        }
    }
    else {
        for (int x = 2; x >= placement.x; --x) {
            if (heights[x] >= stack) {
                return true;
            }
        }
    }

    if (placement.x + direction::get_offset_x(placement.r) > 2) {
        for (int x = 2; x <= placement.x + direction::get_offset_x(placement.r); ++x) {
            if (heights[x] >= stack) {
                return true;
            }
        }
    }
    else {
        for (int x = 2; x >= placement.x + direction::get_offset_x(placement.r); --x) {
            if (heights[x] >= stack) {
                return true;
            }
        }
    }

    return false;
};

// Converts all 180-move in the input queue to real game inputs
Queue Finder::get_queue_convert_m180(Queue& queue)
{
    Queue result = Queue();

    for (i32 i = 0; i < queue.size(); ++i) {
        if (queue[i] != Input::M180) {
            result.push_back(queue[i]);
            continue;
        }

        // Checks the rotation direction
        Input m180_rotate_type = Input::CW;

        for (i32 k = i; k < queue.size(); ++k) {
            if (queue[k] == Input::CW) {
                m180_rotate_type = Input::CCW;
                break;
            }

            if (queue[k] == Input::CCW) {
                m180_rotate_type = Input::CW;
                break;
            }
        }

        // Adds double rotations for move-180
        result.push_back(m180_rotate_type);
        result.push_back(Input::NONE);
        result.push_back(m180_rotate_type);

        // Adds 8 frames buffer
        // We have to wait 8 frames for the 180 rotation animation to finish before we can continue moving the pair
        result.push_back(Input::LEFT);
        result.push_back(Input::NONE);
        result.push_back(Input::LEFT);
        result.push_back(Input::NONE);
        result.push_back(Input::LEFT);
        result.push_back(Input::NONE);
        result.push_back(Input::LEFT);
        result.push_back(Input::NONE);
    }

    return result;
};

// Finds movement path but with movement cancelation
Queue Finder::find_cancel(Field& field, move::Placement placement, cell::Pair pair)
{
    // Simple drop
    if (placement.x == 2 && placement.r == direction::Type::UP) {
        Queue drop;
        drop.push_back(Input::DROP);
        return drop;
    }

    // If we are placing pair on the 9th row then we don't try to cancel
    if (Finder::above_stack_move(field, placement, 9)) {
        return {};
    }

    u8 heights[6];
    field.get_heights(heights);

    // Find horizontal movement cancelation
    // Place vertically
    auto cancel_mv_ver = Finder::cancel_movement_vertical(heights, placement, pair);

    if (!cancel_mv_ver.empty()) {
        return cancel_mv_ver;
    }

    // Place horizontally
    auto cancel_mv_hor = Finder::cancel_movement_horizontal(heights, placement, pair);

    if (!cancel_mv_hor.empty()) {
        return cancel_mv_hor;
    }

    return {};
};

Queue Finder::cancel_movement_horizontal(u8 height[6], move::Placement placement, cell::Pair pair)
{
    if (placement.r == direction::Type::UP || placement.r == direction::Type::DOWN) {
        return {};
    }

    if (pair.first == pair.second) {
        if (placement.r == direction::Type::RIGHT && placement.x <= 2) {
            placement.x += 1;
            placement.r = direction::Type::LEFT;
        }
        else if (placement.r == direction::Type::LEFT && placement.x >= 2) {
            placement.x -= 1;
            placement.r = direction::Type::RIGHT;
        }
    }

    if (placement.x == 2) {
        return {};
    }

    if (placement.r == direction::Type::RIGHT) {
        // Left
        if (placement.x < 2 && height[placement.x] == height[placement.x + 1] && height[placement.x] >= height[placement.x + 2]) {
            auto sim = placement;

            while (true)
            {
                if (height[sim.x + 2] > height[sim.x]) {
                    break;
                }

                sim.x += 1;

                if (sim.x == 2 || height[sim.x] > height[sim.x + 1]) {
                    break;
                }
            }

            path::Queue queue;

            for (i32 i = 0; i < 2 - sim.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::LEFT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::LEFT);
            }

            queue.push_back(path::Input::CW);
            queue.push_back(path::Input::TOUCH);

            for (i32 i = 0; i < sim.x - placement.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::LEFT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::LEFT);
            }
            
            queue.push_back(path::Input::DROP);

            return queue;
        }

        // Right
        if (placement.x > 2 && height[placement.x] == height[placement.x + 1] && height[placement.x] >= height[placement.x - 1]) {
            auto sim = placement;

            while (true)
            {
                if (height[sim.x - 1] > height[sim.x]) {
                    break;
                }

                sim.x -= 1;

                if (sim.x == 2 || height[sim.x] < height[sim.x + 1]) {
                    break;
                }
            }

            path::Queue queue;

            for (i32 i = 0; i < sim.x - 2; ++i) {
                if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::RIGHT);
            }

            queue.push_back(path::Input::CW);
            queue.push_back(path::Input::TOUCH);

            for (i32 i = 0; i < placement.x - sim.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::RIGHT);
            }

            queue.push_back(path::Input::DROP);

            return queue;
        }
    }

    if (placement.r == direction::Type::LEFT) {
        // Left
        if (placement.x < 2 && height[placement.x] == height[placement.x - 1] && height[placement.x] >= height[placement.x + 1]) {
            auto sim = placement;

            while (true)
            {
                if (height[sim.x + 1] > height[sim.x]) {
                    break;
                }

                sim.x += 1;

                if (sim.x == 2 || height[sim.x] < height[sim.x - 1]) {
                    break;
                }
            }

            path::Queue queue;

            for (i32 i = 0; i < 2 - sim.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::LEFT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::LEFT);
            }

            queue.push_back(path::Input::CCW);
            queue.push_back(path::Input::TOUCH);

            for (i32 i = 0; i < sim.x - placement.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::LEFT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::LEFT);
            }

            queue.push_back(path::Input::DROP);

            return queue;
        }

        // Right
        if (placement.x > 2 && height[placement.x] == height[placement.x - 1] && height[placement.x] >= height[placement.x - 2]) {
            auto sim = placement;

            while (true)
            {
                if (height[sim.x - 2] > height[sim.x]) {
                    break;
                }

                sim.x -= 1;

                if (sim.x == 2 || height[sim.x] > height[sim.x - 1]) {
                    break;
                }
            }

            path::Queue queue;

            for (i32 i = 0; i < sim.x - 2; ++i) {
                if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::RIGHT);
            }

            queue.push_back(path::Input::CCW);
            queue.push_back(path::Input::TOUCH);

            for (i32 i = 0; i < placement.x - sim.x; ++i) {
                if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::RIGHT);
            }

            queue.push_back(path::Input::DROP);

            return queue;
        }
    }

    return {};
};

Queue Finder::cancel_movement_vertical(u8 height[6], move::Placement placement, cell::Pair pair)
{
    if (placement.r == direction::Type::RIGHT || placement.r == direction::Type::LEFT) {
        return {};
    }

    if (placement.x == 2) {
        return {};
    }

    // Left
    if (placement.x < 2) {
        auto sim = placement;

        while (true)
        {
            if (height[sim.x + 1] > height[sim.x] + 1) {
                break;
            }

            sim.x += 1;

            if (sim.x == 2 || height[sim.x] < height[sim.x - 1] || height[sim.x] == height[sim.x - 1] + 1) {
                break;
            }
        }

        bool step = height[sim.x] == height[sim.x - 1] + 1;

        if (sim.x == placement.x) {
            return {};
        }

        path::Queue queue;

        if (placement.r == direction::Type::DOWN || step) {
            queue.push_back(path::Input::CCW);
        }

        for (i32 i = 0; i < 2 - sim.x; ++i) {
            if (!queue.empty() && queue.back() == path::Input::LEFT) {
                queue.push_back(path::Input::NONE);
            }

            queue.push_back(path::Input::LEFT);
        }

        if (step) {
            queue.push_back(path::Input::TOUCH);
            
            if (placement.r == direction::Type::UP) {
                queue.push_back(path::Input::CW);
            }
            else {
                queue.push_back(path::Input::CCW);
            }

            queue.push_back(path::Input::TOUCH);
            queue.push_back(path::Input::NONE);
            queue.push_back(path::Input::WAIT);
            queue.push_back(path::Input::LEFT);
            queue.push_back(path::Input::NONE);

            sim.x -= 1;
        }
        else {
            if (placement.r == direction::Type::DOWN) {
                if (queue.back() == path::Input::CCW) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::CCW);
            }

        }

        queue.push_back(path::Input::TOUCH);

        for (i32 i = 0; i < sim.x - placement.x; ++i) {
            if (!queue.empty() && queue.back() == path::Input::LEFT) {
                queue.push_back(path::Input::NONE);
            }

            queue.push_back(path::Input::LEFT);
        }

        queue.push_back(path::Input::DROP);

        return queue;
    }

    // Right
    if (placement.x > 2) {
        auto sim = placement;

        while (true)
        {
            if (height[sim.x - 1] > height[sim.x] + 1) {
                break;
            }

            sim.x -= 1;

            if (sim.x == 2 || height[sim.x] < height[sim.x + 1] || height[sim.x] == height[sim.x + 1] + 1) {
                break;
            }
        }

        bool step = height[sim.x] == height[sim.x + 1] + 1;

        if (sim.x == placement.x) {
            return {};
        }

        path::Queue queue;

        if (placement.r == direction::Type::DOWN || step) {
            queue.push_back(path::Input::CW);
        }

        for (i32 i = 0; i < sim.x - 2; ++i) {
            if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                queue.push_back(path::Input::NONE);
            }

            queue.push_back(path::Input::RIGHT);
        }

        if (step) {
            queue.push_back(path::Input::TOUCH);

            if (placement.r == direction::Type::UP) {
                queue.push_back(path::Input::CCW);
            }
            else {
                queue.push_back(path::Input::CW);
            }

            queue.push_back(path::Input::TOUCH);
            queue.push_back(path::Input::NONE);
            queue.push_back(path::Input::WAIT);
            queue.push_back(path::Input::RIGHT);
            queue.push_back(path::Input::NONE);

            sim.x += 1;
        }
        else {
            if (placement.r == direction::Type::DOWN) {
                if (queue.back() == path::Input::CW) {
                    queue.push_back(path::Input::NONE);
                }

                queue.push_back(path::Input::CW);
            }

        }

        queue.push_back(path::Input::TOUCH);

        for (i32 i = 0; i < placement.x - sim.x; ++i) {
            if (!queue.empty() && queue.back() == path::Input::RIGHT) {
                queue.push_back(path::Input::NONE);
            }

            queue.push_back(path::Input::RIGHT);
        }

        queue.push_back(path::Input::DROP);

        return queue;
    }

    return {};
};

};