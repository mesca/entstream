#include <stdint.h>
#include <math.h>

// Lookup table for counting set bits
// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
static const unsigned int BitsSetTable256[256] = {
    #define B2(n) n,     n+1,     n+1,     n+2
    #define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
    #define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

int count_set_bits(uint8_t *bytes, int size) {
    int c = 0;
    for (int i = 0; i < size; i++) {
        c += BitsSetTable256[bytes[i]];
    }
    return c;
}

double z_score(int setbits, int size) {
    double mean = size * 4;
    double std = sqrt(mean * .5);
    return (setbits - mean) / std;
}