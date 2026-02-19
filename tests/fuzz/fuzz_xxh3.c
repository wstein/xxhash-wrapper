#include <stddef.h>
#include <stdint.h>

#include "xxh3.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    uint64_t seed = 0x1f2e3d4c5b6a7988ULL;
    uint64_t scalar = xxh3_64_scalar(data, size, seed);

#if XXH3_HAVE_SSE2
    if (xxh3_64_sse2(data, size, seed) != scalar) return 1;
#endif
#if XXH3_HAVE_AVX2
    if (xxh3_64_avx2(data, size, seed) != scalar) return 1;
#endif
#if XXH3_HAVE_AVX512
    if (xxh3_64_avx512(data, size, seed) != scalar) return 1;
#endif
#if XXH3_HAVE_NEON
    if (xxh3_64_neon(data, size, seed) != scalar) return 1;
#endif
#if XXH3_HAVE_SVE
    if (xxh3_64_sve(data, size, seed) != scalar) return 1;
#endif
    return 0;
}
