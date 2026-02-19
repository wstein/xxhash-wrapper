#include "xxh3.h"

#include <stdlib.h>
#include <string.h>

#include "xxhash.h"
#include "xxh3_converters.h"

/* Vendor prototypes (ensure thin delegates compile even if header marshalling
 * alters internal symbol names). These mirror the vendor API and are only
 * used as direct-delegate call targets. */
extern void XXH3_generateSecret_fromSeed(void* secretBuffer, uint64_t seed);
extern XXH64_hash_t XXH3_64bits_withSecretandSeed(const void* input, size_t length, const void* secret, size_t secretSize, XXH64_hash_t seed);
extern XXH128_hash_t XXH3_128bits_withSecretandSeed(const void* input, size_t length, const void* secret, size_t secretSize, XXH64_hash_t seed);

struct xxh3_state_t {
    XXH3_state_t* state;
};

static inline xxh3_128_t xxh3_convert_128(XXH128_hash_t value)
{
    return xxh128_to_xxh3(value);
}

// xxh3_64_scalar and xxh3_128_scalar are now in src/variants/scalar.c

uint64_t xxh3_64(const void* input, size_t size, uint64_t seed)
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return xxh3_64_neon(input, size, seed);
#elif defined(__x86_64__) || defined(_M_X64)
    return xxh3_64_avx2(input, size, seed);
#else
    return xxh3_64_scalar(input, size, seed);
#endif
}

xxh3_128_t xxh3_128(const void* input, size_t size, uint64_t seed)
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return xxh3_128_neon(input, size, seed);
#elif defined(__x86_64__) || defined(_M_X64)
    return xxh3_128_avx2(input, size, seed);
#else
    return xxh3_128_scalar(input, size, seed);
#endif
}

/* Unseeded dispatchers (seed=0 by default) */
uint64_t xxh3_64_unseeded(const void* input, size_t size)
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return xxh3_64_neon_unseeded(input, size);
#elif defined(__x86_64__) || defined(_M_X64)
    return xxh3_64_avx2_unseeded(input, size);
#else
    return xxh3_64_scalar_unseeded(input, size);
#endif
}

xxh3_128_t xxh3_128_unseeded(const void* input, size_t size)
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return xxh3_128_neon_unseeded(input, size);
#elif defined(__x86_64__) || defined(_M_X64)
    return xxh3_128_avx2_unseeded(input, size);
#else
    return xxh3_128_scalar_unseeded(input, size);
#endif
}

xxh3_state_t* xxh3_createState(void)
{
    xxh3_state_t* wrapper_state = (xxh3_state_t*)calloc(1, sizeof(*wrapper_state));
    if (wrapper_state == NULL) {
        return NULL;
    }
    wrapper_state->state = XXH3_createState();
    if (wrapper_state->state == NULL) {
        free(wrapper_state);
        return NULL;
    }
    return wrapper_state;
}

void xxh3_freeState(xxh3_state_t* state)
{
    if (state == NULL) {
        return;
    }
    if (state->state != NULL) {
        (void)XXH3_freeState(state->state);
        state->state = NULL;
    }
    free(state);
}

void xxh3_64_reset(xxh3_state_t* state, uint64_t seed)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_64bits_reset_withSeed(state->state, seed);
}

/* Unseeded streaming reset (seed=0 by default) */
void xxh3_64_reset_unseeded(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_64bits_reset_withSeed(state->state, 0);
}

int xxh3_64_update(xxh3_state_t* state, const void* input, size_t size)
{
    if (state == NULL || state->state == NULL) {
        return XXH3_ERROR;
    }
    return (int)XXH3_64bits_update(state->state, input, size);
}

uint64_t xxh3_64_digest(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        return 0;
    }
    return XXH3_64bits_digest(state->state);
}

void xxh3_128_reset(xxh3_state_t* state, uint64_t seed)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_128bits_reset_withSeed(state->state, seed);
}

/* Unseeded streaming reset (seed=0 by default) */
void xxh3_128_reset_unseeded(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_128bits_reset_withSeed(state->state, 0);
}

int xxh3_128_update(xxh3_state_t* state, const void* input, size_t size)
{
    if (state == NULL || state->state == NULL) {
        return XXH3_ERROR;
    }
    return (int)XXH3_128bits_update(state->state, input, size);
}

xxh3_128_t xxh3_128_digest(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        xxh3_128_t zero = {0, 0};
        return zero;
    }
    return xxh3_convert_128(XXH3_128bits_digest(state->state));
}

uint64_t xxh3_64_withSecret(const void* input, size_t size, const void* secret, size_t secretSize)
{
    return XXH3_64bits_withSecret(input, size, secret, secretSize);
}

xxh3_128_t xxh3_128_withSecret(const void* input, size_t size, const void* secret, size_t secretSize)
{
    return xxh3_convert_128(XXH3_128bits_withSecret(input, size, secret, secretSize));
}

void xxh3_64_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_64bits_reset_withSecret(state->state, secret, secretSize);
}

void xxh3_128_reset_withSecret(xxh3_state_t* state, const void* secret, size_t secretSize)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH3_128bits_reset_withSecret(state->state, secret, secretSize);
}

void xxh3_generateSecret(void* secretBuffer, size_t secretSize, uint64_t seed)
{
    unsigned char* out;
    size_t i;
    if (secretBuffer == NULL || secretSize == 0) {
        return;
    }
    out = (unsigned char*)secretBuffer;
    for (i = 0; i < secretSize; i++) {
        out[i] = (unsigned char)((seed >> ((i % 8u) * 8u)) & 0xFFu);
        out[i] ^= (unsigned char)(i * 131u);
    }
}

/* Re-export vendor delegates (thin wrappers) */
void xxh3_generateSecret_fromSeed(void* secretBuffer, uint64_t seed)
{
    /* delegate directly to vendor implementation */
    XXH3_generateSecret_fromSeed(secretBuffer, seed);
}

uint64_t xxh3_64_withSecretandSeed(const void* input, size_t size,
                                    const void* secret, size_t secretSize,
                                    uint64_t seed)
{
    return XXH3_64bits_withSecretandSeed(input, size, secret, secretSize, seed);
}

xxh3_128_t xxh3_128_withSecretandSeed(const void* input, size_t size,
                                       const void* secret, size_t secretSize,
                                       uint64_t seed)
{
    return xxh3_convert_128(XXH3_128bits_withSecretandSeed(input, size, secret, secretSize, seed));
} 

/* ============================================
   State Cloning: Copy a hash state (FR-023)
   ============================================ */

int xxh3_copyState(xxh3_state_t* dst, const xxh3_state_t* src)
{
    if (src == NULL || src->state == NULL) {
        return XXH3_ERROR;
    }
    if (dst == NULL) {
        return XXH3_ERROR;
    }
    if (dst->state == NULL) {
        /* Destination state not initialized; try to allocate */
        dst->state = XXH3_createState();
        if (dst->state == NULL) {
            return XXH3_ERROR;
        }
    }
    /* Copy vendor state */
    XXH3_copyState(dst->state, src->state);
    return XXH3_OK;
}

/* ============================================
   XXH32: Legacy 32-bit hash
   ============================================ */

uint32_t xxh32(const void* input, size_t size, uint32_t seed)
{
    return XXH32(input, size, seed);
}

void xxh32_reset(xxh3_state_t* state, uint32_t seed)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH32_reset((XXH32_state_t*)state->state, seed);
}

int xxh32_update(xxh3_state_t* state, const void* input, size_t size)
{
    if (state == NULL || state->state == NULL) {
        return XXH3_ERROR;
    }
    return (int)XXH32_update((XXH32_state_t*)state->state, input, size);
}

uint32_t xxh32_digest(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        return 0;
    }
    return XXH32_digest((XXH32_state_t*)state->state);
}

/* ============================================
   XXH64: Traditional 64-bit hash
   ============================================ */

uint64_t xxh64(const void* input, size_t size, uint64_t seed)
{
    return XXH64(input, size, seed);
}

void xxh64_reset(xxh3_state_t* state, uint64_t seed)
{
    if (state == NULL || state->state == NULL) {
        return;
    }
    (void)XXH64_reset((XXH64_state_t*)state->state, seed);
}

int xxh64_update(xxh3_state_t* state, const void* input, size_t size)
{
    if (state == NULL || state->state == NULL) {
        return XXH3_ERROR;
    }
    return (int)XXH64_update((XXH64_state_t*)state->state, input, size);
}

uint64_t xxh64_digest(xxh3_state_t* state)
{
    if (state == NULL || state->state == NULL) {
        return 0;
    }
    return XXH64_digest((XXH64_state_t*)state->state);
}




/* ============================================
   XXH128 Comparison Utilities (Optional)
   ============================================ */

int xxh3_128_isEqual(xxh3_128_t h1, xxh3_128_t h2)
{
    return XXH128_isEqual(xxh3_to_xxh128(h1), xxh3_to_xxh128(h2));
}

int xxh3_128_cmp(const void* h128_1, const void* h128_2)
{
    return XXH128_cmp(h128_1, h128_2);
}

/* ============================================
   XXH32 Canonical Representation (Optional)
   ============================================ */

void xxh32_canonicalFromHash(xxh32_canonical_t* dst, uint32_t hash)
{
    if (dst == NULL) {
        return;
    }
    XXH32_canonicalFromHash((XXH32_canonical_t*)dst, hash);
}

uint32_t xxh32_hashFromCanonical(const xxh32_canonical_t* src)
{
    if (src == NULL) {
        return 0;
    }
    return XXH32_hashFromCanonical((const XXH32_canonical_t*)src);
}

/* ============================================
   XXH64 Canonical Representation (Optional)
   ============================================ */

void xxh64_canonicalFromHash(xxh64_canonical_t* dst, uint64_t hash)
{
    if (dst == NULL) {
        return;
    }
    XXH64_canonicalFromHash((XXH64_canonical_t*)dst, hash);
}

uint64_t xxh64_hashFromCanonical(const xxh64_canonical_t* src)
{
    if (src == NULL) {
        return 0;
    }
    return XXH64_hashFromCanonical((const XXH64_canonical_t*)src);
}

/* ============================================
   XXH128 Canonical Representation (Optional)
   ============================================ */

void xxh128_canonicalFromHash(xxh128_canonical_t* dst, xxh3_128_t hash)
{
    XXH128_hash_t v;
    if (dst == NULL) {
        return;
    }
    /* map wrapper fields to vendor type */
    v.low64 = hash.low;
    v.high64 = hash.high;
    XXH128_canonicalFromHash((XXH128_canonical_t*)dst, v);
}

xxh3_128_t xxh128_hashFromCanonical(const xxh128_canonical_t* src)
{
    xxh3_128_t out = {0, 0};
    XXH128_hash_t v;
    if (src == NULL) {
        return out;
    }
    v = XXH128_hashFromCanonical((const XXH128_canonical_t*)src);
    out.high = v.high64;
    out.low  = v.low64;
    return out;
}
