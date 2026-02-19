#include "xxh3.h"

#include <stdlib.h>
#include <string.h>

#include "xxhash.h"

struct xxh3_state_t {
    XXH3_state_t* state;
};

static xxh3_128_t xxh3_convert_128(XXH128_hash_t value)
{
    xxh3_128_t out;
    out.high = value.high64;
    out.low = value.low64;
    return out;
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
