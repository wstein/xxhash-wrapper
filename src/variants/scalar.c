#include "xxh3.h"

#define XXH_VECTOR XXH_SCALAR
#define XXH_INLINE_ALL
#include "xxhash.h"

#include "xxh3_converters.h"
#include "common/internal_utils.h"

uint64_t xxh3_64_scalar(const void* input, size_t size, uint64_t seed)
{
    XXH3_WRAPPER_GUARD(
        if (input == NULL && size > 0) {
            return 0;
        }
    );
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_scalar(const void* input, size_t size, uint64_t seed)
{
    XXH3_WRAPPER_GUARD(
        if (input == NULL && size > 0) {
            xxh3_128_t zero;
            zero.high = 0;
            zero.low  = 0;
            return zero;
        }
    );
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, seed));
}

/* Unseeded variants (default seed=0) */
uint64_t xxh3_64_scalar_unseeded(const void* input, size_t size)
{
    XXH3_WRAPPER_GUARD(
        if (input == NULL && size > 0) {
            return 0;
        }
    );
    return XXH3_64bits_withSeed(input, size, 0);
}

xxh3_128_t xxh3_128_scalar_unseeded(const void* input, size_t size)
{
    XXH3_WRAPPER_GUARD(
        if (input == NULL && size > 0) {
            xxh3_128_t zero;
            zero.high = 0;
            zero.low  = 0;
            return zero;
        }
    );
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, 0));
}
