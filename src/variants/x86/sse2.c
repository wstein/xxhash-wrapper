#include "xxh3.h"

#define XXH_VECTOR XXH_SSE2
#define XXH_INLINE_ALL
#include "xxhash.h"

#include "xxh3_converters.h"

uint64_t xxh3_64_sse2(const void* input, size_t size, uint64_t seed)
{
#ifndef NDEBUG
    if (input == NULL && size > 0) {
        return 0;
    }
#endif
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_sse2(const void* input, size_t size, uint64_t seed)
{
#ifndef NDEBUG
    if (input == NULL && size > 0) {
        xxh3_128_t zero = {0, 0};
        return zero;
    }
#endif
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, seed));
}

/* Unseeded variants (default seed=0) */
uint64_t xxh3_64_sse2_unseeded(const void* input, size_t size)
{
#ifndef NDEBUG
    if (input == NULL && size > 0) {
        return 0;
    }
#endif
    return XXH3_64bits_withSeed(input, size, 0);
}

xxh3_128_t xxh3_128_sse2_unseeded(const void* input, size_t size)
{
#ifndef NDEBUG
    if (input == NULL && size > 0) {
        xxh3_128_t zero = {0, 0};
        return zero;
    }
#endif
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, 0));
}
