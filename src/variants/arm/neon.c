#include "xxh3.h"

#define XXH_VECTOR XXH_NEON
#define XXH_INLINE_ALL
#include "xxhash.h"

static xxh3_128_t xxh3_convert_128_neon(XXH128_hash_t value)
{
    xxh3_128_t out;
    out.high = value.high64;
    out.low = value.low64;
    return out;
}

uint64_t xxh3_64_neon(const void* input, size_t size, uint64_t seed)
{
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_neon(const void* input, size_t size, uint64_t seed)
{
    return xxh3_convert_128_neon(XXH3_128bits_withSeed(input, size, seed));
}

/* Unseeded variants (default seed=0) */
uint64_t xxh3_64_neon_unseeded(const void* input, size_t size)
{
    return XXH3_64bits_withSeed(input, size, 0);
}

xxh3_128_t xxh3_128_neon_unseeded(const void* input, size_t size)
{
    return xxh3_convert_128_neon(XXH3_128bits_withSeed(input, size, 0));
}
