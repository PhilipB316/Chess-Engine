#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MOVES 256
#define MOVE_LEN 8

char moves[MAX_MOVES][MOVE_LEN];
int move_count = 0;

void handle_uci() {
    printf("id name MinimalUCI\n");
    printf("id author Copilot\n");
    printf("uciok\n");
}

void handle_isready() {
    printf("readyok\n");
}

void handle_position(char *line) {
    // Example: position startpos moves e2e4 e7e5 g1f3
    move_count = 0;
    char *moves_str = strstr(line, "moves");
    if (moves_str) {
        moves_str += 6; // skip "moves "
        char *token = strtok(moves_str, " \n");
        while (token && move_count < MAX_MOVES) {
            strncpy(moves[move_count++], token, MOVE_LEN - 1);
            moves[move_count - 1][MOVE_LEN - 1] = '\0';
            token = strtok(NULL, " \n");
        }
    }
}

void handle_go() {
    // For demonstration, output a fixed move or random move
    const char *demo_moves[] = {"e2e4", "d2d4", "g1f3", "c2c4"};
    int n = sizeof(demo_moves) / sizeof(demo_moves[0]);
    srand((unsigned int)time(NULL));
    int idx = rand() % n;
    printf("bestmove %s\n", demo_moves[idx]);
}

int main() {
    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "uci", 3) == 0) {
            handle_uci();
        } else if (strncmp(line, "isready", 7) == 0) {
            handle_isready();
        } else if (strncmp(line, "position", 8) == 0) {
            handle_position(line);
        } else if (strncmp(line, "go", 2) == 0) {
            handle_go();
        } else if (strncmp(line, "quit", 4) == 0) {
            break;
        }
        fflush(stdout);
    }
    return 0;
}
