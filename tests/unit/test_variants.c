#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "xxh3.h"

static int check_64(const char* input, uint64_t seed)
{
    size_t size = strlen(input);
    uint64_t scalar = xxh3_64_scalar(input, size, seed);
    if (xxh3_64_sse2(input, size, seed) != scalar) return 1;
    if (xxh3_64_avx2(input, size, seed) != scalar) return 1;
    if (xxh3_64_avx512(input, size, seed) != scalar) return 1;
    if (xxh3_64_neon(input, size, seed) != scalar) return 1;
    if (xxh3_64_sve(input, size, seed) != scalar) return 1;
    return 0;
}

static int check_streaming(const char* input, uint64_t seed)
{
    xxh3_state_t* state64 = xxh3_createState();
    xxh3_state_t* state128 = xxh3_createState();
    xxh3_128_t digest128;
    if (state64 == NULL || state128 == NULL) {
        xxh3_freeState(state64);
        xxh3_freeState(state128);
        return 1;
    }

    xxh3_64_reset(state64, seed);
    xxh3_128_reset(state128, seed);
    if (xxh3_64_update(state64, input, strlen(input)) != XXH3_OK) {
        xxh3_freeState(state64);
        xxh3_freeState(state128);
        return 1;
    }
    if (xxh3_128_update(state128, input, strlen(input)) != XXH3_OK) {
        xxh3_freeState(state64);
        xxh3_freeState(state128);
        return 1;
    }

    (void)xxh3_64_digest(state64);
    digest128 = xxh3_128_digest(state128);
    if (digest128.high == 0 && digest128.low == 0) {
        xxh3_freeState(state64);
        xxh3_freeState(state128);
        return 1;
    }

    xxh3_freeState(state64);
    xxh3_freeState(state128);
    return 0;
}

int main(void)
{
    const char* input = "xxh3-wrapper-test";
    uint64_t seed = 0x1234567890abcdefULL;

    if (check_64(input, seed) != 0) {
        fprintf(stderr, "64-bit variant mismatch\n");
        return 1;
    }
    if (check_streaming(input, seed) != 0) {
        fprintf(stderr, "streaming mismatch\n");
        return 1;
    }
    return 0;
}
