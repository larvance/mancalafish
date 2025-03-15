#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include "engine.h"

#define mul5(x) (((x) << 2) + (x))
#define mul6(x) (((x) << 2) + ((x) << 1))

void print_bits(uint64_t num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        putchar((num & (1 << i)) ? '1' : '0');
        if (i % 5 == 0) putchar(' ');
    }
    putchar('\n');
}

// value: [0, 31]
void set_pit(MancalaState *state, int index, uint8_t value) {
    int index5 = mul5(index);
    state->pits &= ~(0b11111ULL << index5);
    state->pits |= ((uint64_t) value << index5);
}

uint8_t get_pit(const MancalaState *state, int index) {
    return (state->pits >> mul5(index)) & 0b11111;
}

// value: [0, 31]
void add_to_pit(MancalaState *state, int index, uint8_t value) {
    state->pits += (uint64_t) value << mul5(index);
}

void set_home_pit(MancalaState *state, int index, uint8_t value) {
    state->home_pits &= ~(0b111111 << mul6(index));
    state->home_pits |= ((uint16_t) (value & 0b111111) << mul6(index));
}

uint8_t get_home_pit(const MancalaState *state, int index) {
    return (state->home_pits >> mul6(index)) & 0b111111;
}

void add_to_home_pit(MancalaState *state, int index, uint8_t value) {
    state->home_pits += (uint16_t) (value & 0b111111) << mul6(index);
}

void reset_state(MancalaState *state) {
    state->pits = 0b001000010000100001000010000100001000010000100001000010000100;
    state->home_pits = 0;
}

void printf_state(const MancalaState *state) {
    uint8_t home1 = get_home_pit(state, 0);
    uint8_t home2 = get_home_pit(state, 1);

    printf("Home B   ");
    for (int i = 0; i <= 5; i++) {
        printf("%2d  ", get_pit(state, i));
    }
    printf("Home A\n");

    printf("   (%d)   ", home2);
    for (int i = 11; i >= 6; i--) {
        printf("%2d  ", get_pit(state, i));
    }
    printf("(%d)\n", home1);
}

void print_state(const MancalaState *state) {
    print_bits(state->pits);
    print_bits(state->home_pits);
}

void check_end(MancalaState *state, int turn) {
    int start = turn * 6;
    int end = start + 6;

    int cleaned = 1;

    for (int i = start; i < end; i++) {
        if (get_pit(state, i) != 0) {
            cleaned = 0;
            break;
        }
    }

    start = 6 - start;
    end = start + 6;

    if (cleaned == 0) {
        for (int i = start; i < end; i++) {
            if (get_pit(state, i) != 0) return; // both rows have at least 1
        }
    }

    int accumulated = 0;

    for (int i = start; i < end; i++) {
        accumulated += get_pit(state, i);
    }

    state->pits = 0;

    add_to_home_pit(state, turn, accumulated);
}

// assuming valid move
int make_move(MancalaState *state, int index, int turn) {
    uint8_t stones = get_pit(state, index);

    if (stones == 1) {
        set_pit(state, index, 0);
        int is_11 = index == 11;
        if (index == 5 || is_11) {
            add_to_home_pit(state, is_11, 1);
            check_end(state, turn);
            return turn;
        }

        add_to_pit(state, index + 1, 1);
    } else {
        set_pit(state, index, 1);
    }

    stones--;
    int pos = index;
    int offset = 0;

    while (stones > 0) {
        pos = (pos + 1) % 14;
        stones--;

        if (pos == 6) {
            add_to_home_pit(state, 0, 1);
            offset = -1;
        } else if (pos == 13) {
            add_to_home_pit(state, 1, 1);
            offset = 0;
        } else {
            add_to_pit(state, pos + offset, 1);
        }
    }

    if ((pos == 6 && !turn) || (pos == 13 && turn)) return turn;
    if (pos % 7 == 6) return !turn;

    int pos_off = pos + offset;

    uint8_t end_stones = get_pit(state, pos_off);

    if ((pos > 6) != turn && end_stones % 2 == 0) {
        set_pit(state, pos_off, 0);
        add_to_home_pit(state, turn, end_stones); // impossible for game to end in this case

        return !turn;
    }

    uint8_t across_stones;
    int across = 11 - pos_off;

    if (end_stones == 1 && (across_stones = get_pit(state, across)) != 0) {
        set_pit(state, across, 0);
        set_pit(state, pos_off, 0);
        add_to_home_pit(state, turn, end_stones + across_stones);
        check_end(state, turn);
    }

    return !turn;
}

int heuristic_evaluate(const MancalaState *state) {
    return get_home_pit(state, 0) - get_home_pit(state, 1);
}

int evaluate(MancalaState state, int depth, int alpha, int beta, int turn) { // NOLINT(*-no-recursion)
    if (depth == 0 || state.pits == 0) {
        return heuristic_evaluate(&state);
    }

    int bestValue = turn ? INT_MIN : INT_MAX;

    int start = turn * 6;
    int end = start + 6;

    for (int i = start; i < end; i++) {
        if (get_pit(&state, i) == 0) continue;

        MancalaState newState = state;
        int nextTurn = make_move(&newState, i, turn);

        int value = evaluate(newState, depth - 1, alpha, beta, nextTurn);

        if (turn == 1) {
            if (value > bestValue) bestValue = value;
            if (value > alpha) alpha = value;
        } else {
            if (value < bestValue) bestValue = value;
            if (value < beta) beta = value;
        }

        if (beta <= alpha) break;
    }

    return bestValue;
}

int get_best_move(MancalaState state, int depth, int turn, int *eval) {
    int bestMove = -1;
    int bestValue = turn ? INT_MIN : INT_MAX;

    int start = turn * 6;
    int end = start + 6;

    for (int i = start; i < end; i++) {
        if (get_pit(&state, i) == 0) continue;

        MancalaState newState = state;
        int nextTurn = make_move(&newState, i, turn);

        int moveValue = evaluate(newState, depth - 1, INT_MIN, INT_MAX, nextTurn);

        if ((turn == 1 && moveValue > bestValue) || (turn == 0 && moveValue < bestValue)) {
            bestValue = moveValue;
            bestMove = i;
        }
    }

    *eval = bestValue;

    return bestMove;
}