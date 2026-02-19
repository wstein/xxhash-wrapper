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
    run_bench("sse2", xxh3_64_sse2, data, size);
    run_bench("avx2", xxh3_64_avx2, data, size);
    run_bench("avx512", xxh3_64_avx512, data, size);
    run_bench("neon", xxh3_64_neon, data, size);
    run_bench("sve", xxh3_64_sve, data, size);

    free(data);
    return 0;
}
