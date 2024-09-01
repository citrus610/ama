#include "gaze.h"

namespace Gaze
{

Data gaze(Field& field, Attack::Result& asearch, i32 frame_offset)
{
    Data result = Data();

    i32 field_count = field.get_count();

    auto cmp_return = [&] (const Attack::Data& a, const Attack::Data& b) {
        return a.score < b.score;
    };

    const i32 REDUNDANCY_MAX = 5;

    auto harass_condition = [&] (Attack::Data& attack) -> bool {
        attack.redundancy = Gaze::get_redundancy(field, attack.result);

        if (attack.redundancy > REDUNDANCY_MAX) {
            return false;
        }

        return attack.count < 4;
    };

    auto defence_1dub_condition = [&] (Attack::Data& attack) -> bool {
        attack.redundancy = Gaze::get_redundancy(field, attack.result);

        if (attack.redundancy > REDUNDANCY_MAX) {
            return false;
        }

        return attack.count <= 3 && attack.frame <= 2 && attack.result.get_count() >= std::max(24, field_count / 2);
    };

    auto defence_2dub_condition = [&] (Attack::Data& attack) -> bool {
        attack.redundancy = Gaze::get_redundancy(field, attack.result);

        if (attack.redundancy > REDUNDANCY_MAX) {
            return false;
        }

        return attack.count <= 5 && attack.frame <= 4 && attack.result.get_count() >= std::max(24, field_count / 2);
    };

    auto defence_3dub_condition = [&] (Attack::Data& attack) -> bool {
        attack.redundancy = Gaze::get_redundancy(field, attack.result);

        if (attack.redundancy > REDUNDANCY_MAX) {
            return false;
        }

        return attack.count <= 7 && attack.frame <= 6 && attack.result.get_count() >= std::max(24, field_count / 2);
    };

    for (auto& c : asearch.candidates) {
        for (auto& attack : c.attacks) {
            attack.frame += frame_offset;
            attack.frame_real += frame_offset;

            if (harass_condition(attack)) {
                result.harass.push_back(attack);
            }

            if (attack.score > result.main_fast.score) {
                result.main_fast = attack;
            }

            if (defence_1dub_condition(attack)) {
                if (result.defence_1dub.count == 0) {
                    result.defence_1dub = attack;
                }

                result.defence_1dub = std::max(
                    result.defence_1dub,
                    attack,
                    cmp_return
                );
            }

            if (defence_2dub_condition(attack)) {
                if (result.defence_2dub.count == 0) {
                    result.defence_2dub = attack;
                }

                result.defence_2dub = std::max(
                    result.defence_2dub,
                    attack,
                    cmp_return
                );
            }

            if (defence_3dub_condition(attack)) {
                if (result.defence_3dub.count == 0) {
                    result.defence_3dub = attack;
                }

                result.defence_3dub = std::max(
                    result.defence_3dub,
                    attack,
                    cmp_return
                );
            }
        }

        for (auto& attack : c.attacks_detect) {
            attack.frame += frame_offset;
            attack.frame_real += frame_offset;

            if (harass_condition(attack)) {
                result.harass.push_back(attack);
            }

            if (attack.score > result.main_fast.score) {
                result.main_fast = attack;
            }

            if (defence_1dub_condition(attack)) {
                if (result.defence_1dub.count == 0) {
                    result.defence_1dub = attack;
                }

                result.defence_1dub = std::max(
                    result.defence_1dub,
                    attack,
                    cmp_return
                );
            }

            if (defence_2dub_condition(attack)) {
                if (result.defence_2dub.count == 0) {
                    result.defence_2dub = attack;
                }

                result.defence_2dub = std::max(
                    result.defence_2dub,
                    attack,
                    cmp_return
                );
            }

            if (defence_3dub_condition(attack)) {
                if (result.defence_3dub.count == 0) {
                    result.defence_3dub = attack;
                }

                result.defence_3dub = std::max(
                    result.defence_3dub,
                    attack,
                    cmp_return
                );
            }
        }
    }

    Quiet::search(field, 2, 2, [&] (Quiet::Result q) {
        auto attack = Attack::Data {
            .count = q.chain,
            .score = q.score,
            .score_total = q.score,
            .frame = 3 + std::max(0, i32(q.plan.get_count()) - field_count),
            .frame_real = 4 + std::max(0, i32(q.plan.get_count()) - field_count),
            .all_clear = false,
            .result = Field()
        };

        attack.frame += frame_offset;
        attack.frame_real += frame_offset;

        if (harass_condition(attack)) {
            result.harass.push_back(attack);
        }

        if (defence_1dub_condition(attack)) {
            if (result.defence_1dub.count == 0) {
                result.defence_1dub = attack;
            }

            result.defence_1dub = std::max(
                result.defence_1dub,
                attack,
                cmp_return
            );
        }

        if (defence_2dub_condition(attack)) {
            if (result.defence_2dub.count == 0) {
                result.defence_2dub = attack;
            }

            result.defence_2dub = std::max(
                result.defence_2dub,
                attack,
                cmp_return
            );
        }

        if (defence_3dub_condition(attack)) {
            if (result.defence_3dub.count == 0) {
                result.defence_3dub = attack;
            }

            result.defence_3dub = std::max(
                result.defence_3dub,
                attack,
                cmp_return
            );
        }
    });

    Quiet::Result q_best = Quiet::Result();

    Quiet::search(field, 8, 3, [&] (Quiet::Result q) {
        if (q.score > q_best.score) {
            q_best = q;
        }
    });

    result.main = Attack::Data {
        .count = q_best.chain,
        .score = q_best.score,
        .score_total = q_best.score,
        .frame = (i32(q_best.plan.get_count()) - field_count) * 3 / 2,
        .frame_real = 1 + (i32(q_best.plan.get_count()) - field_count) * 3 / 2,
        .all_clear = false,
        .result = Field()
    };

    result.main.frame += frame_offset;
    result.main.frame_real += frame_offset;

    return result;
};

i32 get_unburied_count(Field& field)
{
    auto mask = field.get_mask();
    auto mask_empty = ~mask;
    auto mask_color = mask & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)]);
    auto mask_above = (mask_empty | mask_color).get_mask_group(2, 11);
    auto mask_unburied = mask_above & mask_color;

    return mask_unburied.get_count();
};

i32 get_accept_limit(Field& field)
{
    i32 result = 0;

    i32 field_count = field.get_count();
    i32 field_count_left = field.get_height(0) + field.get_height(1) + field.get_height(2);

    if (field.get_height(2) > 9) {
        return 0;
    }

    if (field_count >= 48) {
        result = 0;
    }
    else if (field_count >= 36) {
        result = 6;
    }
    else if (field_count >= 30) {
        if (field.get_height(2) <= 4) {
            result = 9;
        }
        else {
            result = 6;
        }
    }
    else {
        if (field.get_height(2) <= 1) {
            result = 30;
        }
        else {
            result = 18;
        }
    }

    i32 garbage_count = field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();

    return std::max(0, result - garbage_count);
};

i32 get_redundancy(Field& pre, Field& now)
{
    i32 result = 0;

    u8 heights_pre[6];
    u8 heights_now[6];

    pre.get_heights(heights_pre);
    now.get_heights(heights_now);

    i32 heights[6];
    
    for (i32 i = 0; i < 6; ++i) {
        heights[i] = std::min(heights_pre[i], heights_now[i]);
    }

    for (i32 x = 0; x < 6; ++x) {
        for (i32 y = 0; y < heights[x]; ++y) {
            auto pre_cell = pre.get_cell(x, y);

            if (!now.data[static_cast<i32>(pre_cell)].get_bit(x, y)) {
                result += heights[x] - y;
                break;
            }
        }
    }

    return result;
};

bool is_garbage_obstruct(Field& field, Chain::Score detect_highest)
{
    i32 unburied_count = Gaze::get_unburied_count(field);
    i32 garbage_count = field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();
    i32 empty_count = (~field.get_mask()).get_mask_group(2, 11).get_count();

    if (garbage_count < 1) {
        return false;
    }

    return
        (garbage_count >= 18) ||
        (garbage_count >= (field.get_count() / 2)) ||
        (garbage_count >= 12 && unburied_count <= garbage_count) ||
        (garbage_count >= 12 && detect_highest.score <= 630);
};

bool is_small_field(Field& field, Field& other)
{
    i32 field_count = (field.get_mask() & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();
    i32 other_count = (other.get_mask() & (~other.data[static_cast<i32>(Cell::Type::GARBAGE)])).get_count();

    return (other_count >= field_count * 2 || field_count <= 24) && other_count >= 22;
};

i32 get_resource_balance(Field& field, Field& other)
{
    i32 unburied_count_field = Gaze::get_unburied_count(field);
    i32 unburied_count_other = Gaze::get_unburied_count(other);

    i32 field_count = field.get_count();
    i32 other_count = other.get_count();

    if (std::abs(unburied_count_field - unburied_count_other) >= 12) {
        return unburied_count_field - unburied_count_other;
    }

    return field_count - other_count;
};

};