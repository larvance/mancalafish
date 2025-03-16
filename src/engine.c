#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include "engine.h"
#include "utils.h"

#define mul5(x) (((x) << 2) + (x))
#define mul6(x) (((x) << 2) + ((x) << 1))

#if USE_OPENMP == 1
#include <omp.h>
#endif

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
    int cleaned = 1;

    for (int i = 0; i <= 5; i++) {
        if (get_pit(state, i) != 0) {
            cleaned = 0;
            break;
        }
    }

    if (cleaned == 0) {
        for (int i = 6; i <= 11; i++) {
            if (get_pit(state, i) != 0) return; // both rows have at least 1
        }
    }

    int accumulated = 0;

    for (int i = 0; i <= 11; i++) {
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
        int wanted = turn ? 11 : 5;
        if (index == wanted) {
            add_to_home_pit(state, turn, 1);
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

        if (pos == 6) {
            offset = -1;
            if (turn == 1) continue;
            add_to_home_pit(state, 0, 1);
        } else if (pos == 13) {
            offset = stones == 1 ? -2 : 0;
            if (turn == 0) continue;
            add_to_home_pit(state, 1, 1);
        } else {
            add_to_pit(state, pos + offset, 1);
        }

        stones--;
    }

    if ((pos == 6 && !turn) || (pos == 13 && turn)) return turn;
    if (pos % 7 == 6) return !turn;

    int pos_off = pos + offset;

    uint8_t end_stones = get_pit(state, pos_off);

    if ((pos > 6) != turn && end_stones % 2 == 0) {
        set_pit(state, pos_off, 0);
        add_to_home_pit(state, turn, end_stones);
        check_end(state, turn);
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

int position_counter = 0;

int evaluate(MancalaState state, int depth, int alpha, int beta, int turn) { // NOLINT(*-no-recursion)
    if (depth == 0 || state.pits == 0) {
        return heuristic_evaluate(&state);
    }

    int bestValue = turn ? INT_MIN : INT_MAX;
    int start = turn * 6;
    int end = start + 6;

    for (int i = start; i < end; i++) {
        if (get_pit(&state, i) == 0) continue;

        position_counter++;
        MancalaState newState = state;
        int nextTurn = make_move(&newState, i, turn);

#if CONTINUOUS_MOVES_AS_ONE_DEPTH == 1
        int nextDepth = depth - (nextTurn != turn);
#else
        int nextDepth = depth - 1;
#endif

        int value = evaluate(newState, nextDepth, alpha, beta, nextTurn);

        if (turn == 1) {
            bestValue = (value > bestValue) ? value : bestValue;
            alpha = (value > alpha) ? value : alpha;
        } else {
            bestValue = (value < bestValue) ? value : bestValue;
            beta = (value < beta) ? value : beta;
        }

        if (beta <= alpha) break;
    }

    return bestValue;
}

int get_best_move(MancalaState state, int depth, int turn, int *eval) {
    int bestMove = -1;
    int bestValue = turn ? INT_MAX : INT_MIN;

    int start = turn * 6;
    int end = start + 6;

#if USE_OPENMP == 1
#pragma omp parallel
    {
        int localBestMove = -1;
        int localBestValue = turn ? INT_MAX : INT_MIN;

#pragma omp for nowait
        for (int i = start; i < end; i++) {
            if (get_pit(&state, i) == 0) continue;

            MancalaState newState = state;
            int nextTurn = make_move(&newState, i, turn);

            int moveValue = evaluate(newState, depth - 1, INT_MIN, INT_MAX, nextTurn);

            if ((turn == 1 && moveValue < localBestValue) || (turn == 0 && moveValue > localBestValue)) {
                localBestValue = moveValue;
                localBestMove = i;
            }
        }

#pragma omp critical
        {
            if ((turn == 1 && localBestValue < bestValue) || (turn == 0 && localBestValue < bestValue)) {
                bestValue = localBestValue;
                bestMove = localBestMove;
            }
        }
    }
#else
    for (int i = start; i < end; i++) {
        if (get_pit(&state, i) == 0) continue;

        MancalaState newState = state;
        int nextTurn = make_move(&newState, i, turn);

        int moveValue = evaluate(newState, depth - 1, INT_MIN, INT_MAX, nextTurn);

        if ((turn == 1 && moveValue < bestValue) || (turn == 0 && moveValue > bestValue)) {
            bestValue = moveValue;
            bestMove = i;
        }
    }
#endif

    *eval = bestValue;
    return bestMove;
}

int fetch_game_counter() {
    int counter = position_counter;
    position_counter = 0;
    return counter;
}

