#ifndef MANCALAFISH_UTILS_H
#define MANCALAFISH_UTILS_H

#include <stdint.h>
#include <stdio.h>

void print_bits(uint64_t num);

#ifdef CONTINUOUS_MOVES_AS_ONE_DEPTH
#define CONTINUOUS_MOVES_AS_ONE_DEPTH 1
#else
#define CONTINUOUS_MOVES_AS_ONE_DEPTH 0
#endif

#ifdef USE_OPENMP
#define USE_OPENMP 1
#else
#define USE_OPENMP 0
#endif

#ifndef DEPTH
#define DEPTH 12
#endif

#endif //MANCALAFISH_UTILS_H