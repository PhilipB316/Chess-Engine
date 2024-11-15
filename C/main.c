#include <stddef.h>
#include "position.h"

int main(void)
{
    char fen[100] = "rnbqk2r/pppppppp/8/8/8/5n2/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position_t position = fen_to_board(fen);
    position.is_white_turn = true;
    print_board(position.board);

    determine_possible_moves(position);
    print_moves(position.moves, position.num_possible_moves);
}