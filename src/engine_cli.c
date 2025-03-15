#include "engine.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    MancalaState state;

    while (1) {
        char position[40];
        scanf("%s", position);

        int parts[16];
        int part_count = 0;
        char *current = position;
        char *end;

        while (part_count < 16) {
            long val = strtol(current, &end, 10);
            if (current == end) {
                return -1;
            }

            parts[part_count++] = (int) val;
            current = end;

            if (*current == '/') {
                current++;
            } else if (part_count < 16) {
                printf("Invalid position.\n");
                return 1;
            }
        }

        for (int j = 0; j < 12; j++) {
            set_pit(&state, j, parts[j]);
        }

        set_home_pit(&state, 0, parts[12]);
        set_home_pit(&state, 1, parts[13]);

        int turn = parts[14];
        int depth = parts[15];

        int eval;
        int move = get_best_move(state, depth, turn, &eval);
        printf("m=%d,e=%d,d=%d\n", move, eval, depth);
    }
}