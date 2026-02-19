/* _POSIX_C_SOURCE 199309L is required for clock_gettime / CLOCK_MONOTONIC
 * on POSIX platforms when compiling with -std=c99. */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#  define _POSIX_C_SOURCE 199309L
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xxh3.h"

static double elapsed(struct timespec start, struct timespec end)
{
    double s = (double)(end.tv_sec - start.tv_sec);
    double ns = (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
    return s + ns;
}

static void run_bench(const char* name, uint64_t (*fn)(const void*, size_t, uint64_t), const unsigned char* data, size_t size)
{
    struct timespec start;
    struct timespec end;
    uint64_t hash = 0;
    int iterations = 500;
    size_t i;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < (size_t)iterations; i++) {
        hash ^= fn(data, size, (uint64_t)i);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%s: %.3f MB/s (hash=%llu)\n", name,
           ((double)size * (double)iterations / (1024.0 * 1024.0)) / elapsed(start, end),
           (unsigned long long)hash);
}

int main(void)
{
    size_t size = 1 << 20;
    unsigned char* data = (unsigned char*)malloc(size);
    if (data == NULL) {
        return 1;
    }
    memset(data, 7, size);

    run_bench("scalar", xxh3_64_scalar, data, size);
#if XXH3_HAVE_SSE2
    run_bench("sse2", xxh3_64_sse2, data, size);
#endif
#if XXH3_HAVE_AVX2
    run_bench("avx2", xxh3_64_avx2, data, size);
#endif
#if XXH3_HAVE_AVX512
    run_bench("avx512", xxh3_64_avx512, data, size);
#endif
#if XXH3_HAVE_NEON
    run_bench("neon", xxh3_64_neon, data, size);
#endif
#if XXH3_HAVE_SVE
    run_bench("sve", xxh3_64_sve, data, size);
#endif

    free(data);
    return 0;
}
