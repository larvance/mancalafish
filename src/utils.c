#include <stdint.h>
#include <stdio.h>

void print_bits(uint64_t num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        putchar((num & (1 << i)) ? '1' : '0');
        if (i % 5 == 0) putchar(' ');
    }
    putchar('\n');
}
