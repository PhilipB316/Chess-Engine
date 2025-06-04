// evaluate.c

#include <stdint.h>

#include "evaluate.h"

int32_t evaluate_position(const Position_t* position)
{
    int32_t score = 0;
    score += position->piece_value_diff * (position->white_to_move ? 1 : -1);

    return score;
}
