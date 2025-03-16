#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include "engine_avx2.h"

#if USE_AVX2 == 1

#include <immintrin.h>

#else

#include <emmintrin.h> // SSE

#endif

#define mul5(x) (((x) << 2) + (x))
#define mul6(x) (((x) << 2) + ((x) << 1))
#define home_pit(turn) ((turn << 3) - turn + 6)

void set_pit_avx2(__m128i *state, uint8_t index, uint8_t value) {
    uint8_t state_array[16];
    _mm_storeu_si128((__m128i *) state_array, *state);

    state_array[index] = value;

    *state = _mm_loadu_si128((__m128i *) state_array);
}

void add_to_pit_avx2(__m128i *state, uint8_t index, uint8_t value) {
    uint8_t state_array[16];
    _mm_storeu_si128((__m128i *) state_array, *state);

    state_array[index] += value;

    *state = _mm_loadu_si128((__m128i *) state_array);
}

uint8_t get_pit_avx2(const __m128i *state, uint8_t index) {
    __m128i index_vec = _mm_set1_epi8((char) index);
    __m128i shuffled = _mm_shuffle_epi8(*state, index_vec);
    return _mm_cvtsi128_si32(shuffled) & 0xFF;
}

void printf_state_avx2(const __m128i *state) {
    uint8_t home1 = get_pit_avx2(state, 6);
    uint8_t home2 = get_pit_avx2(state, 13);

    printf("Home B   ");
    for (int i = 0; i <= 5; i++) {
        printf("%2d  ", get_pit_avx2(state, i));
    }
    printf("Home A\n");

    printf("   (%d)   ", home2);
    for (int i = 12; i >= 7; i--) {
        printf("%2d  ", get_pit_avx2(state, i));
    }
    printf("(%d)\n", home1);
}

void reset_state_avx2(__m128i *state) {
    *state = _mm_setr_epi8(4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0);
}

void check_end_avx2(__m128i *state, int turn) {
    int cleaned = 1;

    for (int i = 0; i <= 5; i++) {
        if (get_pit_avx2(state, i) != 0) {
            cleaned = 0;
            break;
        }
    }

    if (cleaned == 0) {
        for (int i = 7; i <= 12; i++) {
            if (get_pit_avx2(state, i) != 0) return; // both rows have at least 1
        }
    }

    int accumulated = 0;

    for (int i = 0; i <= 12; i++) {
        if (i != 6) accumulated += get_pit_avx2(state, i);
    }

    *state = _mm_setzero_si128();

    add_to_pit_avx2(state, home_pit(turn), accumulated);
}

// assuming valid move
int make_move_avx2(__m128i *state, int index, int turn) {
    uint8_t stones = get_pit_avx2(state, index);

    if (stones == 1) {
        set_pit_avx2(state, index, 0);
        int wanted = turn ? 12 : 5;

        if (index == wanted) {
            add_to_pit_avx2(state, index + 1, 1);
            check_end_avx2(state, turn);
            return turn;
        }
    } else {
        set_pit_avx2(state, index, 1);
        stones--;
    }

    int pos = index;
    int unwanted = turn ? 6 : 13;

    while (stones > 0) {
        pos = (pos + 1) % 14;
        if (pos == unwanted) pos = (pos + 1) % 14;

        stones--;

        add_to_pit_avx2(state, pos, 1);
    }

    if ((pos == 6 && !turn) || (pos == 13 && turn)) return turn;
    if (pos % 7 == 6) return !turn;

    uint8_t end_stones = get_pit_avx2(state, pos);

    if ((pos > 6) != turn && end_stones % 2 == 0) {
        set_pit_avx2(state, pos, 0);
        add_to_pit_avx2(state, home_pit(turn), end_stones);
        check_end_avx2(state, turn);
        return !turn;
    }

    uint8_t across_stones;
    int across = 12 - pos;

    if (end_stones == 1 && (pos > 6) == turn && (across_stones = get_pit_avx2(state, across)) != 0) {
        set_pit_avx2(state, across, 0);
        set_pit_avx2(state, pos, 0);
        add_to_pit_avx2(state, home_pit(turn), end_stones + across_stones);
        check_end_avx2(state, turn);
    }

    return !turn;
}

int heuristic_evaluate_avx2(const __m128i *state) {
    return get_pit_avx2(state, 6) - get_pit_avx2(state, 13);
}

int position_counter_avx2 = 0;

int evaluate_avx2(__m128i state, int depth, int alpha, int beta, int turn) { // NOLINT(*-no-recursion)
    if (depth == 0 || _mm_testz_si128(state, state)) { // if zero, game is over
        return heuristic_evaluate_avx2(&state);
    }

    int bestValue = turn ? INT_MIN : INT_MAX;
    int start = turn * 7;
    int end = start + 6;

    for (int i = start; i < end; i++) {
        if (get_pit_avx2(&state, i) == 0) continue;

        position_counter_avx2++;
        __m128i newState = state;
        int nextTurn = make_move_avx2(&newState, i, turn);

#if CONTINUOUS_MOVES_AS_ONE_DEPTH == 1
        int nextDepth = depth - (nextTurn != turn);
#else
        int nextDepth = depth - 1;
#endif

        int value = evaluate_avx2(newState, nextDepth, alpha, beta, nextTurn);

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

int get_best_move_avx2(__m128i state, int depth, int turn, int *eval) {
    int bestMove = -1;
    int bestValue = turn ? INT_MAX : INT_MIN;

    int start = turn * 6;
    int end = start + 6;

    for (int i = start; i < end; i++) {
        if (get_pit_avx2(&state, i) == 0) continue;

        __m128i newState = state;
        int nextTurn = make_move_avx2(&newState, i, turn);

        int moveValue = evaluate_avx2(newState, depth - 1, INT_MIN, INT_MAX, nextTurn);

        if ((turn == 1 && moveValue < bestValue) || (turn == 0 && moveValue > bestValue)) {
            bestValue = moveValue;
            bestMove = i;
        }
    }

    *eval = bestValue;
    return bestMove;
}

int fetch_game_counter_avx2() {
    int counter = position_counter_avx2;
    position_counter_avx2 = 0;
    return counter;
}