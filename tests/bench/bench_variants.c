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
#include <signal.h>
#include <setjmp.h>

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
    int iterations = 1000; /* increased for stability */
    size_t i;

    /* warmup: run for ~500ms to let CPU frequency scaling settle */
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        for (i = 0; i < 10000; i++) {
            hash ^= fn(data, size, (uint64_t)i);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
    } while (elapsed(start, end) < 0.5);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < (size_t)iterations; i++) {
        hash ^= fn(data, size, (uint64_t)i);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%-10s: %.3f MB/s (hash=%llu)\n", name,
           ((double)size * (double)iterations / (1024.0 * 1024.0)) / elapsed(start, end),
           (unsigned long long)hash);
}

static void run_bench32(const char* name, uint32_t (*fn)(const void*, size_t, uint32_t), const unsigned char* data, size_t size)
{
    struct timespec start;
    struct timespec end;
    uint32_t hash = 0;
    int iterations = 1000;
    size_t i;

    /* warmup: run for ~500ms to let CPU frequency scaling settle */
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        for (i = 0; i < 10000; i++) {
            hash ^= fn(data, size, (uint32_t)i);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
    } while (elapsed(start, end) < 0.5);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < (size_t)iterations; i++) {
        hash ^= fn(data, size, (uint32_t)i);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%-10s: %.3f MB/s (hash=%u)\n", name,
           ((double)size * (double)iterations / (1024.0 * 1024.0)) / elapsed(start, end),
           (unsigned int)hash);
}

/* Generic signal guard for SIMD variants that may raise SIGILL or SIGSEGV on
 * CPUs that do not support the target instruction set (e.g. AVX-512 on older
 * x86, SVE on ARM cores that lack it, NEON where absent).
 * The handler and jmpbuf are always compiled so there is no risk of an
 * "unused function" warning regardless of which SIMD macros are defined. */
static sigjmp_buf _bench_jmpbuf;
static volatile sig_atomic_t _bench_caught_sig;

static void _bench_sig_handler(int sig)
{
    _bench_caught_sig = sig;
    siglongjmp(_bench_jmpbuf, 1);
}

/* RUN_BENCH_SAFE(variant_name, fn_ptr, call_expr):
 *   Executes call_expr inside a SIGILL/SIGSEGV guard.
 *   If the CPU does not support the required instructions the signal is caught
 *   and a "not supported" line is printed instead of crashing. 
 *   If fn_ptr is NULL (weak symbol not found), it skips with a specific message. */
#define RUN_BENCH_SAFE(variant_name, fn_ptr, call_expr) do {                    \
    if ((void*)(fn_ptr) == NULL) {                                              \
        printf("%-10s: not compiled/linked into this binary, skipping\n",       \
               variant_name);                                                   \
        break;                                                                  \
    }                                                                           \
    struct sigaction _act, _oldill, _oldsegv;                                   \
    memset(&_act, 0, sizeof(_act));                                              \
    _act.sa_handler = _bench_sig_handler;                                       \
    sigemptyset(&_act.sa_mask);                                                  \
    _act.sa_flags = 0;                                                           \
    _bench_caught_sig = 0;                                                       \
    sigaction(SIGILL,  &_act, &_oldill);                                        \
    sigaction(SIGSEGV, &_act, &_oldsegv);                                       \
    if (sigsetjmp(_bench_jmpbuf, 1) == 0) {                                     \
        call_expr;                                                               \
    } else {                                                                     \
        printf("%-10s: not supported on this CPU (signal %d), skipping\n",      \
               variant_name, (int)_bench_caught_sig);                            \
    }                                                                            \
    sigaction(SIGILL,  &_oldill,  NULL);                                        \
    sigaction(SIGSEGV, &_oldsegv, NULL);                                       \
} while (0)

/* Force-declare all SIMD variants so we can call them unconditionally in the
 * benchmark and rely on the RUN_BENCH_SAFE signal guard to skip unsupported 
 * ones on the current CPU. 
 * On platforms where these symbols are not available in the binary, we use 
 * weak symbols to avoid linker errors. 
 * Note: macOS linker (ld64) requires symbols that are not present in any 
 * linked library to be declared as either defined in some other TBD file or 
 * we must use ifdefs to at least avoid cross-architecture references. 
 * We use simple ifdefs here to skip variants that we *know* cannot exist 
 * for the current architecture at compile-time. */
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
extern uint64_t xxh3_64_sse2(const void* input, size_t size, uint64_t seed);
extern uint64_t xxh3_64_avx2(const void* input, size_t size, uint64_t seed);
extern uint64_t xxh3_64_avx512(const void* input, size_t size, uint64_t seed);
#define HAVE_X86_VARIANTS 1
#else
#define xxh3_64_sse2   NULL
#define xxh3_64_avx2   NULL
#define xxh3_64_avx512 NULL
#define HAVE_X86_VARIANTS 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
extern uint64_t xxh3_64_neon(const void* input, size_t size, uint64_t seed);
extern uint64_t xxh3_64_sve(const void* input, size_t size, uint64_t seed);
#define HAVE_ARM_VARIANTS 1
#else
#define xxh3_64_neon NULL
#define xxh3_64_sve  NULL
#define HAVE_ARM_VARIANTS 0
#endif

int main(void)
{
    size_t size = 100 * 1024; /* 100 KB for more stable benchmarks */
    unsigned char* data = (unsigned char*)malloc(size);
    if (data == NULL) {
        return 1;
    }
    memset(data, 7, size);

    /* scalar is always safe — no guard needed */
    printf("--- XXH3 64-bit Variants ---\n");
    run_bench("scalar", xxh3_64_scalar, data, size);

    RUN_BENCH_SAFE("neon",   xxh3_64_neon,   run_bench("neon",   xxh3_64_neon,   data, size));
    RUN_BENCH_SAFE("sve",    xxh3_64_sve,    run_bench("sve",    xxh3_64_sve,    data, size));
    RUN_BENCH_SAFE("sse2",   xxh3_64_sse2,   run_bench("sse2",   xxh3_64_sse2,   data, size));
    RUN_BENCH_SAFE("avx2",   xxh3_64_avx2,   run_bench("avx2",   xxh3_64_avx2,   data, size));
    RUN_BENCH_SAFE("avx512", xxh3_64_avx512, run_bench("avx512", xxh3_64_avx512, data, size));

    printf("\n--- Legacy XXH32 / XXH64 (Scalar) ---\n");
    run_bench32("xxh32", xxh32, data, size);
    run_bench("xxh64", xxh64, data, size);

    free(data);
    return 0;
}
