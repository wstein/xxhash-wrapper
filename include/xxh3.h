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

/* Compile-time version string: "MAJOR.MINOR.PATCH.WRAPPER_PATCH" */
#define XXH3_WRAPPER_VERSION_STRING "0.8.3.0"

/* Minimum required secret size (bytes) for secret-based XXH3 hashing */
#define XXH3_SECRET_SIZE_MIN 136

#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
/* Compile-time assertion that a secret buffer meets the minimum size */
#  define XXH3_ASSERT_SECRET_SIZE(secretSize) \
    _Static_assert((secretSize) >= XXH3_SECRET_SIZE_MIN, \
        "Secret buffer must be at least XXH3_SECRET_SIZE_MIN (136) bytes")
#else
#  define XXH3_ASSERT_SECRET_SIZE(secretSize) ((void)0)
#endif

/* Platform-specific variant availability (FR-005) */
/* x86-64: SSE2, AVX2, AVX512 variants are always available */
/* aarch64: NEON, SVE variants are available */
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#  define XXH3_HAVE_X86_SIMD 1
#  define XXH3_HAVE_SSE2     1
#  define XXH3_HAVE_AVX2     1
#  define XXH3_HAVE_AVX512   1
#else
#  define XXH3_HAVE_X86_SIMD 0
#  define XXH3_HAVE_SSE2     0
#  define XXH3_HAVE_AVX2     0
#  define XXH3_HAVE_AVX512   0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#  define XXH3_HAVE_AARCH64_SIMD 1
#  define XXH3_HAVE_NEON   1
#  define XXH3_HAVE_SVE    1  /* Always export on aarch64; consumer handles CPU feature detection */
#else
#  define XXH3_HAVE_AARCH64_SIMD 0
#  define XXH3_HAVE_NEON   0
#  define XXH3_HAVE_SVE    0
#endif

/*!
 * @brief The return value from 128-bit hashes.
 *
 * Stored in little endian order, although the fields themselves are in native
 * endianness.
 *
 * Do NOT rely on raw memory copies of `XXH128_hash_t` for cross-platform or
 * on-the-wire serialization. For a machine-independent (canonical)
 * representation use `xxh128_canonicalFromHash()` and
 * `xxh128_hashFromCanonical()` which produce/consume a fixed big-endian 16‑byte
 * form.
 */
typedef struct {
    uint64_t high;
    uint64_t low;
} xxh3_128_t;

typedef struct xxh3_state_t xxh3_state_t;

#define XXH3_OK 0
#define XXH3_ERROR 1

/* Generic dispatcher functions (e.g. `xxh3_64`, `xxh3_128` and their
 * unseeded counterparts) were removed from the wrapper. Consumers must call
 * per-variant functions directly (for example: `xxh3_64_scalar()`,
 * `xxh3_64_avx2()`, `xxh3_128_neon_unseeded()`, etc.) or implement their own
 * dispatch logic.
 *
 * Per-variant prototypes are declared below (e.g. `xxh3_64_scalar()`). */

uint64_t xxh3_64_scalar(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_scalar_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_scalar(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_scalar_unseeded(const void* input, size_t size);

/* x86-64 SIMD variants (always available on x86-64 builds) */
#if XXH3_HAVE_SSE2
uint64_t xxh3_64_sse2(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_sse2_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_sse2(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_sse2_unseeded(const void* input, size_t size);
#endif

#if XXH3_HAVE_AVX2
uint64_t xxh3_64_avx2(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_avx2_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_avx2(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_avx2_unseeded(const void* input, size_t size);
#endif

#if XXH3_HAVE_AVX512
uint64_t xxh3_64_avx512(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_avx512_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_avx512(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_avx512_unseeded(const void* input, size_t size);
#endif

/* aarch64 SIMD variants (only available on aarch64 builds) */
#if XXH3_HAVE_NEON
uint64_t xxh3_64_neon(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_neon_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_neon(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_neon_unseeded(const void* input, size_t size);
#endif

#if XXH3_HAVE_SVE
uint64_t xxh3_64_sve(const void* input, size_t size, uint64_t seed);
uint64_t xxh3_64_sve_unseeded(const void* input, size_t size);
xxh3_128_t xxh3_128_sve(const void* input, size_t size, uint64_t seed);
xxh3_128_t xxh3_128_sve_unseeded(const void* input, size_t size);
#endif

xxh3_state_t* xxh3_createState(void);
void xxh3_freeState(xxh3_state_t* state);

void xxh3_64_reset(xxh3_state_t* state, uint64_t seed);
void xxh3_64_reset_unseeded(xxh3_state_t* state);
int xxh3_64_update(xxh3_state_t* state, const void* input, size_t size);
uint64_t xxh3_64_digest(xxh3_state_t* state);

void xxh3_128_reset(xxh3_state_t* state, uint64_t seed);
void xxh3_128_reset_unseeded(xxh3_state_t* state);
int xxh3_128_update(xxh3_state_t* state, const void* input, size_t size);
xxh3_128_t xxh3_128_digest(xxh3_state_t* state);

uint64_t xxh3_64_withSecret(const void* input, size_t size, const void* secret, size_t secretSize);
xxh3_128_t xxh3_128_withSecret(const void* input, size_t size, const void* secret, size_t secretSize);
void xxh3_64_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize);
void xxh3_128_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize);
void xxh3_generateSecret(void* secretBuffer, size_t secretSize, uint64_t seed);
#define XXH3_generateSecret(secretBuffer, secretSize, seed) \
    xxh3_generateSecret((secretBuffer), (secretSize), (seed))

/* State copying: Clone a hash state for branching computation (FR-023) */
int xxh3_copyState(xxh3_state_t* dst, const xxh3_state_t* src);

/* XXH32 and XXH64: scalar-only single-shot functions (no SIMD variants) */
uint32_t xxh32(const void* input, size_t size, uint32_t seed);
uint64_t xxh64(const void* input, size_t size, uint64_t seed);

/* XXH32 and XXH64: streaming APIs (using shared xxh3_state_t) */
void xxh32_reset(xxh3_state_t* state, uint32_t seed);
int xxh32_update(xxh3_state_t* state, const void* input, size_t size);
uint32_t xxh32_digest(xxh3_state_t* state);

void xxh64_reset(xxh3_state_t* state, uint64_t seed);
int xxh64_update(xxh3_state_t* state, const void* input, size_t size);
uint64_t xxh64_digest(xxh3_state_t* state);

/* XXH3 Advanced: Secret AND Seed (delegates to vendor) */
uint64_t xxh3_64_withSecretandSeed(const void* input, size_t size,
                                   const void* secret, size_t secretSize,
                                   uint64_t seed);
xxh3_128_t xxh3_128_withSecretandSeed(const void* input, size_t size,
                                      const void* secret, size_t secretSize,
                                      uint64_t seed);

/* XXH3 Secret Generation: Derive secret directly from seed (vendor delegate) */
void xxh3_generateSecret_fromSeed(void* secretBuffer, uint64_t seed);

/* XXH128 Comparison Utilities */
int xxh3_128_isEqual(xxh3_128_t h1, xxh3_128_t h2);
int xxh3_128_cmp(const void* h128_1, const void* h128_2);

/* XXH32 Canonical Representation */
typedef struct {
    unsigned char digest[4];
} xxh32_canonical_t;
void xxh32_canonicalFromHash(xxh32_canonical_t* dst, uint32_t hash);
uint32_t xxh32_hashFromCanonical(const xxh32_canonical_t* src);

/* XXH64 Canonical Representation */
typedef struct {
    unsigned char digest[8];
} xxh64_canonical_t;
void xxh64_canonicalFromHash(xxh64_canonical_t* dst, uint64_t hash);
uint64_t xxh64_hashFromCanonical(const xxh64_canonical_t* src);

/* XXH128 Canonical Representation */

typedef struct {
    unsigned char digest[16];
} xxh128_canonical_t;
void xxh128_canonicalFromHash(xxh128_canonical_t* dst, xxh3_128_t hash);
xxh3_128_t xxh128_hashFromCanonical(const xxh128_canonical_t* src);

/*
 * CPU requirements for variant functions (consumer dispatch responsibility):
 * - xxh3_*_sse2: x86/x64 with SSE2
 * - xxh3_*_avx2: x86/x64 with AVX2
 * - xxh3_*_avx512: x86/x64 with AVX512F (and OS XSAVE enabled)
 * - xxh3_*_neon: aarch64 with NEON
 * - xxh3_*_sve: aarch64 with SVE
 *
 * NOTE: xxh32 and xxh64 have no SIMD variants and can be called unconditionally.
 * Streaming state is shared with XXH3 via xxh3_state_t; lock algorithm at reset time.
 *
 * NOTE: xxh32 and xxh64 do not support secret-based variants;
 * secret randomization is available only for XXH3 family functions.
 */

#ifdef __cplusplus
}
#endif

#endif
