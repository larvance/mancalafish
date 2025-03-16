#include "engine.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

int main() {
    int turn = 0;
    MancalaState state;
    reset_state(&state);
    /*__m128i state;
    reset_state_avx2(&state);*/

#if AI_STARTS_FIRST == 1
    int user_turn = 1;
#else
    int user_turn = 0;
#endif

    while (1) {
        printf_state(&state);

        if (state.pits == 0) {
            int homeA = get_home_pit(&state, 0);
            int homeB = get_home_pit(&state, 1);
            printf("Game over! Home A: %d, Home B: %d\n", homeA, homeB);
            printf("You %s!\n", (homeA > homeB) != user_turn ? "won" : "lost");
            return 0;
        }

        if (turn == user_turn) {
            int player_move = -1;
            while (player_move < 0 || player_move > 5 || get_pit(&state, player_move) == 0) {
                printf("Your move (0-5): ");
                char input[100];
                int r = scanf("%s", input);
                if (r != 1) continue;
                if (strcmp(input, "q") == 0) return 0;
                if (input[0] == '\0' || input[1] != '\0' || input[0] < '0' || input[0] > '5') continue;
                player_move = input[0] - '0';
#if AI_STARTS_FIRST == 1
                player_move = 11 - player_move;
#endif
            }
            turn = make_move(&state, player_move, turn);
        } else {
            clock_t start_time = clock();
            int eval;
            int bestMove = get_best_move(state, DEPTH, turn, &eval);
            turn = make_move(&state, bestMove, turn);
            clock_t end_time = clock();
            double elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
            int counter = fetch_game_counter();
            printf("\n[e=%d,t=%.3fs,d=%d,p=%dM,c=%d,o=%d] AI plays: %d\n", eval, elapsed_time, DEPTH, counter / 1000000,
                   CONTINUOUS_MOVES_AS_ONE_DEPTH, USE_OPENMP, bestMove);
            // [e=-11,t=7.814s,d=12,p=193M,c=1] AI plays: 11
            // [e=-7,t=0.589s,d=12,p=12M,c=0] AI plays: 9
            // [e=-7,t=32.048s,d=16,p=677M,c=0] AI plays: 11
        }
    }
}