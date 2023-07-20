#include "field.h"

Field::Field()
{
    for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
        this->data[cell] = FieldBit();
    }
};

bool Field::operator == (const Field& other)
{
    for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
        if (this->data[cell] != other.data[cell]) {
            return false;
        }
    }

    return true;
};

bool Field::operator != (const Field& other)
{
    return !(*this == other);
};

void Field::set_cell(i8 x, i8 y, Cell::Type cell)
{
    this->data[static_cast<u8>(cell)].set_bit(x, y);
};

Cell::Type Field::get_cell(i8 x, i8 y)
{
    if (x < 0 || x > 5 || y < 0 || y > 12) {
        return Cell::Type::NONE;
    }
    for (u8 i = 0; i < Cell::COUNT; ++i) {
        if (this->data[i].get_bit(x, y)) {
            return Cell::Type(i);
        }
    }
    return Cell::Type::NONE;
};

u32 Field::get_count()
{
    u32 result = 0;
    for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
        result += this->data[cell].get_count();
    }
    return result;
};

u8 Field::get_height(i8 x)
{
    FieldBit mask = this->get_mask();

    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, mask.data);

    return 16 - std::countl_zero(v[x]);
};

u8 Field::get_height_max()
{
    u8 heights[6];
    this->get_heights(heights);
    return *std::max_element(heights, heights + 6);
};

void Field::get_heights(u8 heights[6])
{
    FieldBit mask = this->get_mask();

    alignas(16) u16 v[8];
    _mm_store_si128((__m128i*)v, mask.data);

    for (i32 i = 0; i < 6; ++i) {
        heights[i] = 16 - std::countl_zero(v[i]);
    }
};

FieldBit Field::get_mask()
{
    FieldBit result = FieldBit();
    for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
        result = result | this->data[cell];
    }
    return result;
};

Field Field::get_mask_pop()
{
    Field result = Field();
    for (u8 cell = 0; cell < Cell::COUNT - 1; ++cell) {
        result.data[cell] = this->data[cell].get_mask_pop();
    }
    return result;
};

u8 Field::get_drop_pair_frame(i8 x, Direction::Type direction)
{
    return 1 + (this->get_height(x) != this->get_height(x + Direction::get_offset_x(direction)));
};

bool Field::is_occupied(i8 x, i8 y)
{
    if (x < 0 || x > 5 || y < 0 || y > 12) {
        return true;
    }
    return y < this->get_height(x);
};

bool Field::is_occupied(i8 x, i8 y, u8 heights[6])
{
    if (x < 0 || x > 5 || y < 0 || y > 12) {
        return true;
    }
    return y < heights[x];
};

bool Field::is_colliding_pair(i8 x, i8 y, Direction::Type direction)
{
    u8 heights[6];
    this->get_heights(heights);
    return this->is_colliding_pair(x, y, direction, heights);
};

bool Field::is_colliding_pair(i8 x, i8 y, Direction::Type direction, u8 heights[6])
{
    return
        this->is_occupied(x, y, heights) ||
        this->is_occupied(x + Direction::get_offset_x(direction), y + Direction::get_offset_y(direction), heights);
};

bool Field::is_empty()
{
    for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
        if (!this->data[cell].is_empty()) {
            return false;
        }
    }
    return true;
};

void Field::drop_puyo(i8 x, Cell::Type cell)
{
    assert(x >= 0 && x < 6);
    u8 height = this->get_height(x);
    if (height < 13) {
        this->set_cell(x, height, cell);
    }
};

void Field::drop_pair(i8 x, Direction::Type direction, Cell::Pair pair)
{
    assert(x >= 0 && x < 6);
    switch (direction)
    {
    case Direction::Type::UP:
        this->drop_puyo(x, pair.first);
        this->drop_puyo(x, pair.second);
        break;
    case Direction::Type::RIGHT:
        this->drop_puyo(x, pair.first);
        this->drop_puyo(x + 1, pair.second);
        break;
    case Direction::Type::DOWN:
        this->drop_puyo(x, pair.second);
        this->drop_puyo(x, pair.first);
        break;
    case Direction::Type::LEFT:
        this->drop_puyo(x, pair.first);
        this->drop_puyo(x - 1, pair.second);
        break;
    }
};

avec<Field, 19> Field::pop()
{
    avec<Field, 19> result = avec<Field, 19>();

    for (i32 index = 0; index < 20; ++index) {
        Field pop = this->get_mask_pop();
        FieldBit mask_pop = pop.get_mask();
        if (_mm_testz_si128(mask_pop.data, mask_pop.data)) {
            break;
        }
        result.add(pop);

        mask_pop = mask_pop | (mask_pop.get_expand() & this->data[static_cast<u8>(Cell::Type::GARBAGE)]);
        for (u8 cell = 0; cell < Cell::COUNT; ++cell) {
            this->data[cell].pop(mask_pop);
        }
    }

    return result;
};

void Field::from(const char c[13][7])
{
    *this = Field();
    for (i8 y = 0; y < 13; ++y) {
        for (i8 x = 0; x < 6; ++x) {
            if (c[12 - y][x] == '.' || c[12 - y][x] == ' ') {
                continue;
            }
            this->set_cell(x, y, Cell::from_char(c[12 - y][x]));
        }
    }
};

void Field::print()
{
    using namespace std;
    for (i8 y = 12; y >= 0; --y) {
        for (i8 x = 0; x < 6; ++x) {
            cout << Cell::to_char(this->get_cell(x, y));
        }
        cout << "\n";
    }
    cout << endl;
};