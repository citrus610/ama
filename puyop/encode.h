#include <iostream>
#include "../ai/ai.h"

namespace encode
{

constexpr char CHAR[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]";

constexpr int get_cell_id(cell::Type cell)
{
    switch (cell)
    {
    case cell::Type::RED:
        return 0;
    case cell::Type::GREEN:
        return 1;
    case cell::Type::BLUE:
        return 2;
    case cell::Type::YELLOW:
        return 3;
    }
    return 0;
};

constexpr int get_field_cell_id(cell::Type cell)
{
    switch (cell)
    {
    case cell::Type::NONE:
        return 0;
    case cell::Type::RED:
        return 1;
    case cell::Type::GREEN:
        return 2;
    case cell::Type::BLUE:
        return 3;
    case cell::Type::YELLOW:
        return 4;
    case cell::Type::GARBAGE:
        return 6;
    }
    return 0;
};

inline std::string get_encoded_control(std::vector<cell::Pair> queue, std::vector<move::Placement> placements)
{
    std::string result;

    for (size_t i = 0; i < placements.size(); ++i) {
        auto pair = queue[i];

        i32 pair_code = encode::get_cell_id(pair.first) * 5 + encode::get_cell_id(pair.second);
        i32 placement_code = (i32(placements[i].x + 1) << 2) + static_cast<i32>(placements[i].r);
        i32 code = pair_code | (placement_code << 7);

        result.push_back(encode::CHAR[code & 0x3F]);
        result.push_back(encode::CHAR[(code >> 6) & 0x3F]);
    }

    return result;
};

inline std::string get_encoded_field(Field field)
{
    if (field.is_empty()) {
        return "";
    }

    std::string result;

    bool start = false;

    for (i8 y = 13; y > 0; --y) {
        for (i8 x = 1; x <= 6; x += 2) {
            if (!start && field.get_cell(x, y) == cell::Type::NONE && field.get_cell(x + 1, y) == cell::Type::NONE) {
                continue;
            }

            i32 code = get_field_cell_id(field.get_cell(x, y)) * 8 + get_field_cell_id(field.get_cell(x + 1, y));

            start = true;

            result.push_back(encode::CHAR[code]);
        }
    }

    return result;
};

inline std::string get_encoded_URL(Field field, std::vector<cell::Pair> queue, std::vector<move::Placement> placements)
{
    return std::string("http://www.puyop.com/s/") + encode::get_encoded_field(field) + "_" + encode::get_encoded_control(queue, placements);
};

};