#include "xxh3.h"

#include "xxhash.h"

static xxh3_128_t xxh3_convert_128_sve(XXH128_hash_t value)
{
    xxh3_128_t out;
    out.high = value.high64;
    out.low = value.low64;
    return out;
}

uint64_t xxh3_64_sve(const void* input, size_t size, uint64_t seed)
{
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_sve(const void* input, size_t size, uint64_t seed)
{
    return xxh3_convert_128_sve(XXH3_128bits_withSeed(input, size, seed));
}
