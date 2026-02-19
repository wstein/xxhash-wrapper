#include "xxh3.h"

#define XXH_VECTOR XXH_SVE
#define XXH_INLINE_ALL
#include "xxhash.h"

#include "xxh3_converters.h"

uint64_t xxh3_64_sve(const void* input, size_t size, uint64_t seed)
{
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_sve(const void* input, size_t size, uint64_t seed)
{
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, seed));
}

/* Unseeded variants (default seed=0) */
uint64_t xxh3_64_sve_unseeded(const void* input, size_t size)
{
    return XXH3_64bits_withSeed(input, size, 0);
}

xxh3_128_t xxh3_128_sve_unseeded(const void* input, size_t size)
{
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, 0));
}
