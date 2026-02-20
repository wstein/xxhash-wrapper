#include "xxh3.h"

#define XXH_VECTOR XXH_NEON
#define XXH_INLINE_ALL
#include "xxhash.h"

#include "xxh3_converters.h"
#include "common/internal_utils.h"

uint64_t xxh3_64_neon(const void* input, size_t size, uint64_t seed)
{
    XXH3_WRAPPER_GUARD({
        if (input == NULL && size > 0) {
            return 0;
        }
    });
    return XXH3_64bits_withSeed(input, size, seed);
}

xxh3_128_t xxh3_128_neon(const void* input, size_t size, uint64_t seed)
{
    XXH3_WRAPPER_GUARD({
        if (input == NULL && size > 0) {
            return ((xxh3_128_t){0,0});
        }
    });
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, seed));
}

/* Unseeded variants (default seed=0) */
uint64_t xxh3_64_neon_unseeded(const void* input, size_t size)
{
    XXH3_WRAPPER_GUARD({
        if (input == NULL && size > 0) {
            return 0;
        }
    });
    return XXH3_64bits_withSeed(input, size, 0);
}

xxh3_128_t xxh3_128_neon_unseeded(const void* input, size_t size)
{
    XXH3_WRAPPER_GUARD({
        if (input == NULL && size > 0) {
            return ((xxh3_128_t){0,0});
        }
    });
    return xxh128_to_xxh3(XXH3_128bits_withSeed(input, size, 0));
}
