#ifndef XXH3_WRAPPER_H
#define XXH3_WRAPPER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XXH3_WRAPPER_VERSION_MAJOR 0
#define XXH3_WRAPPER_VERSION_MINOR 8
#define XXH3_WRAPPER_VERSION_PATCH 3
#define XXH3_WRAPPER_VERSION_WRAPPER_PATCH 0

typedef struct {
    uint64_t high;
    uint64_t low;
} xxh3_128_t;

typedef struct xxh3_state_t xxh3_state_t;

#define XXH3_OK 0
#define XXH3_ERROR 1

uint64_t xxh3_64(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128(const void* input, size_t size, uint64_t seed);

uint64_t xxh3_64_scalar(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_scalar(const void* input, size_t size, uint64_t seed);

uint64_t xxh3_64_sse2(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_sse2(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_avx2(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_avx2(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_avx512(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_avx512(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_neon(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_neon(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_sve(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_sve(const void* input, size_t size, uint64_t seed);

xxh3_state_t* xxh3_createState(void);
void xxh3_freeState(xxh3_state_t* state);

void xxh3_64_reset(xxh3_state_t* state, uint64_t seed);
int xxh3_64_update(xxh3_state_t* state, const void* input, size_t size);
uint64_t xxh3_64_digest(xxh3_state_t* state);

void xxh3_128_reset(xxh3_state_t* state, uint64_t seed);
int xxh3_128_update(xxh3_state_t* state, const void* input, size_t size);
xxh3_128_t xxh3_128_digest(xxh3_state_t* state);

uint64_t xxh3_64_withSecret(const void* input, size_t size, const void* secret, size_t secretSize);
xxh3_128_t xxh3_128_withSecret(const void* input, size_t size, const void* secret, size_t secretSize);
void xxh3_64_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize);
void xxh3_128_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize);
void xxh3_generateSecret(void* secretBuffer, size_t secretSize, uint64_t seed);
#define XXH3_generateSecret(secretBuffer, secretSize, seed) \
    xxh3_generateSecret((secretBuffer), (secretSize), (seed))

uint32_t xxh32(const void* input, size_t size, uint32_t seed);
uint64_t xxh64(const void* input, size_t size, uint64_t seed);

/*
 * CPU requirements for variant functions (consumer dispatch responsibility):
 * - xxh3_*_sse2: x86/x64 with SSE2
 * - xxh3_*_avx2: x86/x64 with AVX2
 * - xxh3_*_avx512: x86/x64 with AVX512F (and OS XSAVE enabled)
 * - xxh3_*_neon: aarch64 with NEON
 * - xxh3_*_sve: aarch64 with SVE
 */

#ifdef __cplusplus
}
#endif

#endif
