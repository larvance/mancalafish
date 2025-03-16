#ifndef MANCALAFISH_ENGINE_AVX2_H
#define MANCALAFISH_ENGINE_AVX2_H

#include <stdint.h>
#include <immintrin.h>

void set_pit_avx2(__m128i *state, uint8_t index, uint8_t value);

uint8_t get_pit_avx2(const __m128i *state, uint8_t index);

void add_to_pit_avx2(__m128i *state, uint8_t index, uint8_t value);

void printf_state_avx2(const __m128i *state);

void reset_state_avx2(__m128i *state);

void check_end_avx2(__m128i *state, int turn);

int make_move_avx2(__m128i *state, int index, int turn);

int heuristic_evaluate_avx2(const __m128i *state);

int evaluate_avx2(__m128i state, int depth, int alpha, int beta, int turn);

int get_best_move_avx2(__m128i state, int depth, int turn, int *eval);

int fetch_game_counter_avx2();

#endif //MANCALAFISH_ENGINE_AVX2_H
