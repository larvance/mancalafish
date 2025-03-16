#ifndef MANCALAFISH_ENGINE_H
#define MANCALAFISH_ENGINE_H

#include <stdint.h>

typedef struct {
    uint64_t pits;
    uint16_t home_pits;
} MancalaState;

// value: [0, 31]
void set_pit(MancalaState *state, int index, uint8_t value);

uint8_t get_pit(const MancalaState *state, int index);

// value: [0, 31]
void add_to_pit(MancalaState *state, int index, uint8_t value);

void set_home_pit(MancalaState *state, int index, uint8_t value);

uint8_t get_home_pit(const MancalaState *state, int index);

void add_to_home_pit(MancalaState *state, int index, uint8_t value);

void reset_state(MancalaState *state);

void printf_state(const MancalaState *state);

void print_state(const MancalaState *state);

void check_end(MancalaState *state, int turn);

// assuming valid move
int make_move(MancalaState *state, int index, int turn);

int heuristic_evaluate(const MancalaState *state);

int evaluate(MancalaState state, int depth, int alpha, int beta, int turn);

int get_best_move(MancalaState state, int depth, int turn, int *eval);

int fetch_game_counter();

#endif //MANCALAFISH_ENGINE_H
