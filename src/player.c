#include "engine.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

int main() {
    int turn = 0;
    MancalaState state;
    reset_state(&state);

    while (1) {
        printf_state(&state);

        if (turn == 0) {
            int player_move = -1;
            while (player_move < 0 || player_move > 5 || get_pit(&state, player_move) == 0) {
                printf("Your move (0-5): ");
                char input[100];
                scanf("%s", input);
                if (strcmp(input, "q") == 0) return 0;
                if (strlen(input) != 1 || input[0] < '0' || input[0] > '5') continue;
                player_move = input[0] - '0';
                if (get_pit(&state, player_move) == 0) continue;
            }
            turn = make_move(&state, player_move, turn);
        } else {
            int depth = 16;
            clock_t start_time = clock();
            int eval;
            int bestMove = get_best_move(state, depth, turn, &eval);
            turn = make_move(&state, bestMove, turn);
            clock_t end_time = clock();
            double elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC * 1000;
            printf("\n[e=%d,t=%.0fms,d=%d] AI (%d) plays: %d\n", eval, elapsed_time, depth, turn, bestMove);
        }
    }
}