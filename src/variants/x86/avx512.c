#include "xxh3.h"

#define XXH_VECTOR XXH_AVX512
#define XXH_INLINE_ALL
#include "xxhash.h"

static xxh3_128_t xxh3_convert_128_avx512(XXH128_hash_t value)
{
    xxh3_128_t out;
    out.high = value.high64;
    out.low = value.low64;
    return out;
}

uint64_t xxh3_64_avx512(const void* input, size_t size, uint64_t seed)
{
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_avx512(const void* input, size_t size, uint64_t seed)
{
    return xxh3_convert_128_avx512(XXH3_128bits_withSeed(input, size, seed));
}
