#include <iostream>
#include "../ai/ai.h"

namespace Encode
{

constexpr char CHAR[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]";

constexpr int get_cell_id(Cell::Type cell)
{
    switch (cell)
    {
    case Cell::Type::RED:
        return 0;
    case Cell::Type::GREEN:
        return 1;
    case Cell::Type::BLUE:
        return 2;
    case Cell::Type::YELLOW:
        return 3;
    }
    return 0;
};

constexpr int get_field_cell_id(Cell::Type cell)
{
    switch (cell)
    {
    case Cell::Type::NONE:
        return 0;
    case Cell::Type::RED:
        return 1;
    case Cell::Type::GREEN:
        return 2;
    case Cell::Type::BLUE:
        return 3;
    case Cell::Type::YELLOW:
        return 4;
    case Cell::Type::GARBAGE:
        return 6;
    }
    return 0;
};

static std::string get_encoded_control(std::vector<Cell::Pair> queue, std::vector<Move::Placement> placements)
{
    std::string result;

    for (size_t i = 0; i < placements.size(); ++i) {
        auto pair = queue[i];

        i32 pair_code = Encode::get_cell_id(pair.first) * 5 + Encode::get_cell_id(pair.second);
        i32 placement_code = (i32(placements[i].x + 1) << 2) + static_cast<i32>(placements[i].r);
        i32 code = pair_code | (placement_code << 7);

        result.push_back(Encode::CHAR[code & 0x3F]);
        result.push_back(Encode::CHAR[(code >> 6) & 0x3F]);
    }

    return result;
};

static std::string get_encoded_field(Field field)
{
    if (field.is_empty()) {
        return "";
    }

    std::string result;

    bool start = false;

    for (i8 y = 13; y > 0; --y) {
        for (i8 x = 1; x <= 6; x += 2) {
            if (!start && field.get_cell(x, y) == Cell::Type::NONE && field.get_cell(x + 1, y) == Cell::Type::NONE) {
                continue;
            }

            i32 code = get_field_cell_id(field.get_cell(x, y)) * 8 + get_field_cell_id(field.get_cell(x + 1, y));

            start = true;

            result.push_back(Encode::CHAR[code]);
        }
    }

    return result;
};

static std::string get_encoded_URL(Field field, std::vector<Cell::Pair> queue, std::vector<Move::Placement> placements)
{
    return std::string("http://www.puyop.com/s/") + Encode::get_encoded_field(field) + "_" + Encode::get_encoded_control(queue, placements);
};

};