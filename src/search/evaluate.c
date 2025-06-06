// evaluate.c

#include <stdint.h>

#include "evaluate.h"

int32_t opening_evaluation(const Position_t* position);
int32_t midgame_evaluation(const Position_t* position);

int32_t evaluate_position(const Position_t* position)
{
    if (position->half_move_count < MID_GAME_MOVE_COUNT)
    {
        return opening_evaluation(position);
    } else {
        return midgame_evaluation(position);
    }
}

int32_t opening_evaluation(const Position_t* position)
{
    int32_t score = 0;
    score += position->piece_value_diff * (position->white_to_move ? 1 : -1);

    return score;
}

int32_t midgame_evaluation(const Position_t* position)
{
    int32_t score = 0;
    score += position->piece_value_diff * (position->white_to_move ? 1 : -1);

    return score;
}
