#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"


int main(void)
{
    Move_t move_list[300] = {0};
    size_t num_moves = 0;
    printf("Hello, World!\n");

    char fen[100] = "rnb1kbnr/pp1p1ppp/4pq2/2p5/3P4/2NQ1N2/PPP1PPPP/R1B1KB1R b KQkq - 3 4";
    Position_t position = fen_to_board(fen);
    print_bitboard(position.black_pieces.kings);

    generate_lookup_tables();
    printf("done\n");
    queen_move_finder(move_list, &num_moves, &position);
    print_moves(move_list, &num_moves);
}
