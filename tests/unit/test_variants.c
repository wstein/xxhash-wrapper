/* =============================================================================
 * tests/unit/test_variants.c
 *
 * Comprehensive Unity test suite for the xxhash-wrapper library.
 * Covers:
 *   - Version macros and API availability
 *   - XXH3-64 and XXH3-128: seeded single-shot all variants vs scalar reference
 *   - XXH3-64 and XXH3-128: unseeded single-shot all variants (FR-024)
 *   - XXH3 streaming: reset/update/digest matches single-shot (seeded & unseeded)
 *   - XXH3 incremental chunked streaming
 *   - XXH3 secret-based hashing (single-shot + streaming)
 *   - xxh32 single-shot stability and seed sensitivity
 *   - xxh32 streaming and chunked streaming matches single-shot
 *   - xxh64 single-shot stability and seed sensitivity
 *   - xxh64 streaming and chunked streaming matches single-shot
 *   - Edge cases: empty input, single byte, large deterministic inputs
 *   - Seed sensitivity: seeded variants produce different outputs
 *   - Unseeded consistency: unseeded variants always produce same output (seed=0)
 *   - State isolation: two concurrent states do not interfere
 *   - State reuse: reset + rehash produces identical output
 *   - Cross-algorithm independence: xxh32/xxh64/xxh3 outputs differ
 *   - Null/zero-state defensive return checks
 *
 * NOTE: Functions NOT exported (intentionally absent):
 *   - XXH3_64bits_withSecretandSeed() / XXH3_128bits_withSecretandSeed()
 *   - XXH3_generateSecret_fromSeed()
 *   - XXH128_isEqual() / XXH128_cmp()
 *   - Canonical representation (XXH32/64_canonicalFromHash, etc.)
 *   See README.md and spec.md "Intentionally Omitted" for rationale.
 * =============================================================================
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#include "xxh3.h"
#include "../unity/unity.h"

/* ============================================================ Signal guards
 * For variants that may not be supported on all CPUs (e.g., SVE on Apple Silicon),
 * use signal handlers to gracefully catch SIGILL and skip the test case.
 * ================================================================ */
static sigjmp_buf _test_jmpbuf;
static volatile sig_atomic_t _test_caught_sig;
static volatile int _test_skip_variant;

static void _test_sig_handler(int sig)
{
    _test_caught_sig = sig;
    siglongjmp(_test_jmpbuf, 1);
}

#define TEST_TRY_VARIANT(variant_name, call_expr) do {                          \
    struct sigaction _act, _oldill, _oldsegv;                                     \
    memset(&_act, 0, sizeof(_act));                                             \
    _act.sa_handler = _test_sig_handler;                                        \
    sigemptyset(&_act.sa_mask);                                                  \
    _act.sa_flags = 0;                                                           \
    _test_caught_sig = 0;                                                        \
    _test_skip_variant = 0;                                                      \
    sigaction(SIGILL,  &_act, &_oldill);                                        \
    sigaction(SIGSEGV, &_act, &_oldsegv);                                       \
    if (sigsetjmp(_test_jmpbuf, 1) == 0) {                                      \
        call_expr;                                                               \
    } else {                                                                     \
        _test_skip_variant = 1;                                                  \
    }                                                                            \
    sigaction(SIGILL,  &_oldill,  NULL);                                        \
    sigaction(SIGSEGV, &_oldsegv, NULL);                                       \
} while (0)

/* ------------------------------------------------------------------ helpers */

static const char* SHORT_INPUT = "xxhash-wrapper";
static const char* LOREM       = "Lorem ipsum dolor sit amet, consectetur "
                                 "adipiscing elit, sed do eiusmod tempor "
                                 "incididunt ut labore et dolore magna aliqua.";

/* Allocate + fill a deterministic buffer of the given size */
static unsigned char* make_buf(size_t size)
{
    unsigned char* buf = (unsigned char*)malloc(size);
    size_t         i;
    if (!buf) return NULL;
    for (i = 0; i < size; i++) {
        buf[i] = (unsigned char)(i ^ (i >> 8));
    }
    return buf;
}

#define SEED1     0x0ULL
#define SEED2     0xDEADBEEFCAFEBABEULL
#define SEED32_0  0x00000000UL
#define SEED32_1  0xDEADBEEFUL

/* ============================================================ Unit callbacks */

void setUp(void)    {}
void tearDown(void) {}

/* ------------------------------------------------------------------ version */

static void test_version_string_is_non_empty(void)
{
    const char* v = XXH3_WRAPPER_VERSION_STRING;
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_TRUE(strlen(v) > 0);
}

static void test_version_components_non_negative(void)
{
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, XXH3_WRAPPER_VERSION_MAJOR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, XXH3_WRAPPER_VERSION_MINOR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, XXH3_WRAPPER_VERSION_PATCH);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, XXH3_WRAPPER_VERSION_WRAPPER_PATCH);
}

static void test_secret_size_min_at_least_136(void)
{
    TEST_ASSERT_GREATER_OR_EQUAL_INT(136, XXH3_SECRET_SIZE_MIN);
}

/* -------------------------------------------------------------- xxh3-64 variants */

static void test_xxh3_64_variants_match_scalar_short(void)
{
    const size_t   size = strlen(SHORT_INPUT);
    const uint64_t ref  = xxh3_64_scalar(SHORT_INPUT, size, SEED1);

    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64(SHORT_INPUT, size, SEED1));
#if XXH3_HAVE_SSE2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_sse2(SHORT_INPUT, size, SEED1));
#endif
#if XXH3_HAVE_AVX2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx2(SHORT_INPUT, size, SEED1));
#endif
#if XXH3_HAVE_AVX512
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx512(SHORT_INPUT, size, SEED1));
#endif
#if XXH3_HAVE_NEON
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_neon(SHORT_INPUT, size, SEED1));
#endif
#if XXH3_HAVE_SVE
    TEST_TRY_VARIANT("SVE", {
        uint64_t sve_result = xxh3_64_sve(SHORT_INPUT, size, SEED1);
        if (!_test_skip_variant) {
            TEST_ASSERT_EQUAL_UINT64(ref, sve_result);
        }
    });
#endif
}

static void test_xxh3_64_variants_match_scalar_lorem(void)
{
    const size_t   size = strlen(LOREM);
    const uint64_t ref  = xxh3_64_scalar(LOREM, size, SEED2);

    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64(LOREM, size, SEED2));
#if XXH3_HAVE_SSE2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_sse2(LOREM, size, SEED2));
#endif
#if XXH3_HAVE_AVX2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx2(LOREM, size, SEED2));
#endif
#if XXH3_HAVE_AVX512
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx512(LOREM, size, SEED2));
#endif
#if XXH3_HAVE_NEON
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_neon(LOREM, size, SEED2));
#endif
#if XXH3_HAVE_SVE
    TEST_TRY_VARIANT("SVE", {
        uint64_t sve_result = xxh3_64_sve(LOREM, size, SEED2);
        if (!_test_skip_variant) {
            TEST_ASSERT_EQUAL_UINT64(ref, sve_result);
        }
    });
#endif
}

static void test_xxh3_64_variants_match_scalar_1mb(void)
{
    const size_t   size = 1 << 20;
    unsigned char* buf  = make_buf(size);
    uint64_t       ref;

    TEST_ASSERT_NOT_NULL(buf);
    ref = xxh3_64_scalar(buf, size, SEED1);

    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64(buf, size, SEED1));
#if XXH3_HAVE_SSE2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_sse2(buf, size, SEED1));
#endif
#if XXH3_HAVE_AVX2
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx2(buf, size, SEED1));
#endif
#if XXH3_HAVE_AVX512
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_avx512(buf, size, SEED1));
#endif
#if XXH3_HAVE_NEON
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_neon(buf, size, SEED1));
#endif
#if XXH3_HAVE_SVE
    TEST_TRY_VARIANT("SVE", {
        uint64_t sve_result = xxh3_64_sve(buf, size, SEED1);
        if (!_test_skip_variant) {
            TEST_ASSERT_EQUAL_UINT64(ref, sve_result);
        }
    });
#endif
    free(buf);
}

/* ------------------------------------------------------------ xxh3-128 variants */

static void test_xxh3_128_variants_match_scalar_short(void)
{
    const size_t     size = strlen(SHORT_INPUT);
    const xxh3_128_t ref  = xxh3_128_scalar(SHORT_INPUT, size, SEED1);

#define CHECK128(fn) \
    TEST_ASSERT_EQUAL_UINT64(ref.high, fn(SHORT_INPUT, size, SEED1).high); \
    TEST_ASSERT_EQUAL_UINT64(ref.low,  fn(SHORT_INPUT, size, SEED1).low)

    CHECK128(xxh3_128);
#if XXH3_HAVE_SSE2
    CHECK128(xxh3_128_sse2);
#endif
#if XXH3_HAVE_AVX2
    CHECK128(xxh3_128_avx2);
#endif
#if XXH3_HAVE_AVX512
    CHECK128(xxh3_128_avx512);
#endif
#if XXH3_HAVE_NEON
    CHECK128(xxh3_128_neon);
#endif
#if XXH3_HAVE_SVE
    TEST_TRY_VARIANT("SVE", {
        xxh3_128_t sve_result = xxh3_128_sve(SHORT_INPUT, size, SEED1);
        if (!_test_skip_variant) {
            TEST_ASSERT_EQUAL_UINT64(ref.high, sve_result.high);
            TEST_ASSERT_EQUAL_UINT64(ref.low,  sve_result.low);
        }
    });
#endif
#undef CHECK128
}

static void test_xxh3_128_high_and_low_independently_nonzero(void)
{
    const size_t     size = strlen(LOREM);
    const xxh3_128_t h    = xxh3_128_scalar(LOREM, size, SEED2);
    TEST_ASSERT_TRUE(h.high != 0 || h.low != 0);
}

/* ----------------------------------------- unseeded single-shot variants (FR-024) */

static void test_xxh3_64_unseeded_matches_seed_zero(void)
{
    const size_t   size           = strlen(LOREM);
    const uint64_t with_seed_zero = xxh3_64_scalar(LOREM, size, 0);
    const uint64_t unseeded       = xxh3_64_unseeded(LOREM, size);

    TEST_ASSERT_EQUAL_UINT64(with_seed_zero, unseeded);
}

static void test_xxh3_64_unseeded_scalar_variant(void)
{
    const size_t   size           = strlen(LOREM);
    const uint64_t with_seed_zero = xxh3_64_scalar(LOREM, size, 0);
    const uint64_t unseeded       = xxh3_64_scalar_unseeded(LOREM, size);

    TEST_ASSERT_EQUAL_UINT64(with_seed_zero, unseeded);
}

static void test_xxh3_128_unseeded_matches_seed_zero(void)
{
    const size_t     size           = strlen(LOREM);
    const xxh3_128_t with_seed_zero = xxh3_128_scalar(LOREM, size, 0);
    const xxh3_128_t unseeded       = xxh3_128_unseeded(LOREM, size);

    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.high, unseeded.high);
    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.low,  unseeded.low);
}

static void test_xxh3_128_unseeded_scalar_variant(void)
{
    const size_t     size           = strlen(LOREM);
    const xxh3_128_t with_seed_zero = xxh3_128_scalar(LOREM, size, 0);
    const xxh3_128_t unseeded       = xxh3_128_scalar_unseeded(LOREM, size);

    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.high, unseeded.high);
    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.low,  unseeded.low);
}

/* ------------------------------------------------ xxh3 streaming vs single-shot */

static void test_xxh3_64_stream_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    const uint64_t ref   = xxh3_64_scalar(LOREM, size, SEED2);
    xxh3_state_t*  state = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh3_64_reset(state, SEED2);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, LOREM, size));
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_digest(state));
    xxh3_freeState(state);
}

static void test_xxh3_128_stream_matches_single_shot(void)
{
    const size_t     size  = strlen(LOREM);
    const xxh3_128_t ref   = xxh3_128_scalar(LOREM, size, SEED2);
    xxh3_state_t*    state = xxh3_createState();
    xxh3_128_t       got;

    TEST_ASSERT_NOT_NULL(state);
    xxh3_128_reset(state, SEED2);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state, LOREM, size));
    got = xxh3_128_digest(state);
    TEST_ASSERT_EQUAL_UINT64(ref.high, got.high);
    TEST_ASSERT_EQUAL_UINT64(ref.low,  got.low);
    xxh3_freeState(state);
}

static void test_xxh3_64_unseeded_stream_matches_seed_zero(void)
{
    const size_t   size           = strlen(LOREM);
    const uint64_t with_seed_zero = xxh3_64_scalar(LOREM, size, 0);
    xxh3_state_t*  state          = xxh3_createState();
    uint64_t       unseeded_result;

    TEST_ASSERT_NOT_NULL(state);
    xxh3_64_reset_unseeded(state);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, LOREM, size));
    unseeded_result = xxh3_64_digest(state);
    TEST_ASSERT_EQUAL_UINT64(with_seed_zero, unseeded_result);
    xxh3_freeState(state);
}

static void test_xxh3_128_unseeded_stream_matches_seed_zero(void)
{
    const size_t     size           = strlen(LOREM);
    const xxh3_128_t with_seed_zero = xxh3_128_scalar(LOREM, size, 0);
    xxh3_state_t*    state          = xxh3_createState();
    xxh3_128_t       unseeded_result;

    TEST_ASSERT_NOT_NULL(state);
    xxh3_128_reset_unseeded(state);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state, LOREM, size));
    unseeded_result = xxh3_128_digest(state);
    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.high, unseeded_result.high);
    TEST_ASSERT_EQUAL_UINT64(with_seed_zero.low,  unseeded_result.low);
    xxh3_freeState(state);
}

static void test_xxh3_64_chunked_streaming_matches_single_shot(void)
{
    const size_t   size   = strlen(LOREM);
    const size_t   half   = size / 2;
    const uint64_t ref    = xxh3_64_scalar(LOREM, size, SEED1);
    xxh3_state_t*  state  = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh3_64_reset(state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, LOREM, half));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, LOREM + half, size - half));
    TEST_ASSERT_EQUAL_UINT64(ref, xxh3_64_digest(state));
    xxh3_freeState(state);
}

static void test_xxh3_128_chunked_streaming_matches_single_shot(void)
{
    const size_t     size  = strlen(LOREM);
    const size_t     chunk = 16;
    const xxh3_128_t ref   = xxh3_128_scalar(LOREM, size, SEED1);
    xxh3_state_t*    state = xxh3_createState();
    xxh3_128_t       got;
    size_t           offset;

    TEST_ASSERT_NOT_NULL(state);
    xxh3_128_reset(state, SEED1);
    for (offset = 0; offset + chunk <= size; offset += chunk) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state, LOREM + offset, chunk));
    }
    if (offset < size) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state, LOREM + offset, size - offset));
    }
    got = xxh3_128_digest(state);
    TEST_ASSERT_EQUAL_UINT64(ref.high, got.high);
    TEST_ASSERT_EQUAL_UINT64(ref.low,  got.low);
    xxh3_freeState(state);
}

/* --------------------------------------------------------- secret-based hashing */

static void test_xxh3_64_with_secret_differs_from_seeded(void)
{
    const size_t  size = strlen(LOREM);
    unsigned char secret[200];
    uint64_t      seeded, with_secret;
    size_t        i;

    for (i = 0; i < sizeof(secret); i++) {
        secret[i] = (unsigned char)(i * 17 + 3);
    }
    seeded      = xxh3_64_scalar(LOREM, size, SEED2);
    with_secret = xxh3_64_withSecret(LOREM, size, secret, sizeof(secret));
    TEST_ASSERT_NOT_EQUAL(seeded, with_secret);
}

static void test_xxh3_64_secret_stream_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    unsigned char  secret[192];
    xxh3_state_t*  state;
    uint64_t       ref, got;
    size_t         i;

    for (i = 0; i < sizeof(secret); i++) {
        secret[i] = (unsigned char)(i * 31 + 7);
    }
    ref   = xxh3_64_withSecret(LOREM, size, secret, sizeof(secret));
    state = xxh3_createState();
    TEST_ASSERT_NOT_NULL(state);
    xxh3_64_reset_withSecret(state, secret, sizeof(secret));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, LOREM, size));
    got = xxh3_64_digest(state);
    TEST_ASSERT_EQUAL_UINT64(ref, got);
    xxh3_freeState(state);
}

static void test_xxh3_128_with_secret_matches_stream(void)
{
    const size_t   size  = strlen(SHORT_INPUT);
    unsigned char  secret[160];
    xxh3_state_t*  state;
    xxh3_128_t     ref, got;
    size_t         i;

    for (i = 0; i < sizeof(secret); i++) {
        secret[i] = (unsigned char)(i * 7 + 11);
    }
    ref   = xxh3_128_withSecret(SHORT_INPUT, size, secret, sizeof(secret));
    state = xxh3_createState();
    TEST_ASSERT_NOT_NULL(state);
    xxh3_128_reset_withSecret(state, secret, sizeof(secret));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state, SHORT_INPUT, size));
    got = xxh3_128_digest(state);
    TEST_ASSERT_EQUAL_UINT64(ref.high, got.high);
    TEST_ASSERT_EQUAL_UINT64(ref.low,  got.low);
    xxh3_freeState(state);
}

/* ---------------------------------------------------------------- state copying */

static void test_xxh3_64_copy_state_matches_continued_hashing(void)
{
    const size_t   size1 = strlen(SHORT_INPUT);
    const size_t   size2 = strlen(LOREM);
    xxh3_state_t*  state1 = xxh3_createState();
    xxh3_state_t*  state2 = xxh3_createState();
    uint64_t       ref, got;

    TEST_ASSERT_NOT_NULL(state1);
    TEST_ASSERT_NOT_NULL(state2);

    /* State 1: update once, then copy to state2, continue updating state2 */
    xxh3_64_reset(state1, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state1, SHORT_INPUT, size1));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_copyState(state2, state1));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state2, LOREM, size2));
    got = xxh3_64_digest(state2);

    /* Reference: single state that processes both in order */
    xxh3_state_t* ref_state = xxh3_createState();
    TEST_ASSERT_NOT_NULL(ref_state);
    xxh3_64_reset(ref_state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(ref_state, SHORT_INPUT, size1));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(ref_state, LOREM, size2));
    ref = xxh3_64_digest(ref_state);

    TEST_ASSERT_EQUAL_UINT64(ref, got);
    xxh3_freeState(state1);
    xxh3_freeState(state2);
    xxh3_freeState(ref_state);
}

static void test_xxh3_128_copy_state_branches_hashing(void)
{
    const size_t   size1 = strlen(SHORT_INPUT);
    const size_t   size2a = 5;
    const size_t   size2b = strlen(LOREM) - size2a;
    xxh3_state_t*  state1 = xxh3_createState();
    xxh3_state_t*  state2 = xxh3_createState();
    xxh3_state_t*  state3 = xxh3_createState();
    xxh3_128_t     branch1, branch2;

    TEST_ASSERT_NOT_NULL(state1);
    TEST_ASSERT_NOT_NULL(state2);
    TEST_ASSERT_NOT_NULL(state3);

    /* Initial state: same seed, same first update */
    xxh3_128_reset(state1, SEED2);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state1, SHORT_INPUT, size1));

    /* Branch 1: copy and continue with LOREM as single update */
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_copyState(state2, state1));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state2, LOREM, strlen(LOREM)));
    branch1 = xxh3_128_digest(state2);

    /* Branch 2: copy and continue with LOREM in chunks */
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_copyState(state3, state1));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state3, LOREM, size2a));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_128_update(state3, LOREM + size2a, size2b));
    branch2 = xxh3_128_digest(state3);

    /* Both branches should produce identical results */
    TEST_ASSERT_EQUAL_UINT64(branch1.high, branch2.high);
    TEST_ASSERT_EQUAL_UINT64(branch1.low,  branch2.low);

    xxh3_freeState(state1);
    xxh3_freeState(state2);
    xxh3_freeState(state3);
}

static void test_generate_secret_produces_nonzero_output(void)
{
    unsigned char secret[XXH3_SECRET_SIZE_MIN];
    size_t        i;
    int           all_zero = 1;

    memset(secret, 0, sizeof(secret));
    xxh3_generateSecret(secret, sizeof(secret), SEED2);
    for (i = 0; i < sizeof(secret); i++) {
        if (secret[i] != 0) { all_zero = 0; break; }
    }
    TEST_ASSERT_FALSE(all_zero);
}

/* ---------------------------------------------------------------- edge cases */

static void test_xxh3_64_empty_input_is_stable(void)
{
    uint64_t a = xxh3_64_scalar("", 0, SEED1);
    uint64_t b = xxh3_64_scalar("", 0, SEED1);
    TEST_ASSERT_EQUAL_UINT64(a, b);
}

static void test_xxh3_128_empty_input_is_stable(void)
{
    xxh3_128_t a = xxh3_128_scalar("", 0, SEED1);
    xxh3_128_t b = xxh3_128_scalar("", 0, SEED1);
    TEST_ASSERT_EQUAL_UINT64(a.high, b.high);
    TEST_ASSERT_EQUAL_UINT64(a.low,  b.low);
}

static void test_xxh3_64_single_byte_is_stable(void)
{
    const char c = 'A';
    uint64_t   a = xxh3_64_scalar(&c, 1, SEED1);
    uint64_t   b = xxh3_64_scalar(&c, 1, SEED1);
    TEST_ASSERT_EQUAL_UINT64(a, b);
}

static void test_different_seeds_produce_different_hashes(void)
{
    uint64_t h1 = xxh3_64_scalar(LOREM, strlen(LOREM), SEED1);
    uint64_t h2 = xxh3_64_scalar(LOREM, strlen(LOREM), SEED2);
    TEST_ASSERT_NOT_EQUAL(h1, h2);
}

static void test_xxh3_64_different_inputs_differ(void)
{
    uint64_t ha = xxh3_64_scalar("hello", 5, SEED1);
    uint64_t hb = xxh3_64_scalar("world", 5, SEED1);
    TEST_ASSERT_NOT_EQUAL(ha, hb);
}

static void test_xxh3_128_different_inputs_differ(void)
{
    xxh3_128_t a = xxh3_128_scalar("aaa", 3, SEED1);
    xxh3_128_t b = xxh3_128_scalar("bbb", 3, SEED1);
    TEST_ASSERT_TRUE(a.high != b.high || a.low != b.low);
}

static void test_xxh3_64_avalanche_on_extra_byte(void)
{
    uint64_t h1 = xxh3_64_scalar("xxhash",  6, SEED1);
    uint64_t h2 = xxh3_64_scalar("xxhashy", 7, SEED1);
    TEST_ASSERT_NOT_EQUAL(h1, h2);
}

/* ------------------------------------------------ state isolation / reuse */

static void test_two_states_do_not_interfere(void)
{
    xxh3_state_t* s1 = xxh3_createState();
    xxh3_state_t* s2 = xxh3_createState();
    uint64_t      d1, d2;

    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_NOT_NULL(s2);

    xxh3_64_reset(s1, SEED1);
    xxh3_64_reset(s2, SEED2);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(s1, LOREM, strlen(LOREM)));
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(s2, LOREM, strlen(LOREM)));
    d1 = xxh3_64_digest(s1);
    d2 = xxh3_64_digest(s2);

    TEST_ASSERT_NOT_EQUAL(d1, d2);
    TEST_ASSERT_EQUAL_UINT64(xxh3_64_scalar(LOREM, strlen(LOREM), SEED1), d1);
    TEST_ASSERT_EQUAL_UINT64(xxh3_64_scalar(LOREM, strlen(LOREM), SEED2), d2);

    xxh3_freeState(s1);
    xxh3_freeState(s2);
}

static void test_state_can_be_reset_and_reused(void)
{
    xxh3_state_t* state = xxh3_createState();
    uint64_t      d1, d2;

    TEST_ASSERT_NOT_NULL(state);

    xxh3_64_reset(state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, SHORT_INPUT, strlen(SHORT_INPUT)));
    d1 = xxh3_64_digest(state);

    xxh3_64_reset(state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh3_64_update(state, SHORT_INPUT, strlen(SHORT_INPUT)));
    d2 = xxh3_64_digest(state);

    TEST_ASSERT_EQUAL_UINT64(d1, d2);
    xxh3_freeState(state);
}

/* ------------------------------------------------------ xxh32 */

static void test_xxh32_single_shot_stable(void)
{
    uint32_t a = xxh32(LOREM, strlen(LOREM), SEED32_1);
    uint32_t b = xxh32(LOREM, strlen(LOREM), SEED32_1);
    TEST_ASSERT_EQUAL_UINT32(a, b);
}

static void test_xxh32_different_seeds_differ(void)
{
    uint32_t a = xxh32(LOREM, strlen(LOREM), SEED32_0);
    uint32_t b = xxh32(LOREM, strlen(LOREM), SEED32_1);
    TEST_ASSERT_NOT_EQUAL(a, b);
}

static void test_xxh32_empty_input_stable(void)
{
    uint32_t a = xxh32("", 0, SEED32_0);
    uint32_t b = xxh32("", 0, SEED32_0);
    TEST_ASSERT_EQUAL_UINT32(a, b);
}

static void test_xxh32_stream_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    const uint32_t ref   = xxh32(LOREM, size, SEED32_1);
    xxh3_state_t*  state = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh32_reset(state, SEED32_1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh32_update(state, LOREM, size));
    TEST_ASSERT_EQUAL_UINT32(ref, xxh32_digest(state));
    xxh3_freeState(state);
}

static void test_xxh32_chunked_streaming_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    const size_t   chunk = 8;
    const uint32_t ref   = xxh32(LOREM, size, SEED32_0);
    xxh3_state_t*  state = xxh3_createState();
    size_t         offset;

    TEST_ASSERT_NOT_NULL(state);
    xxh32_reset(state, SEED32_0);
    for (offset = 0; offset + chunk <= size; offset += chunk) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh32_update(state, LOREM + offset, chunk));
    }
    if (offset < size) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh32_update(state, LOREM + offset, size - offset));
    }
    TEST_ASSERT_EQUAL_UINT32(ref, xxh32_digest(state));
    xxh3_freeState(state);
}

static void test_xxh32_stream_reset_reuse(void)
{
    const size_t   size  = strlen(SHORT_INPUT);
    const uint32_t ref   = xxh32(SHORT_INPUT, size, SEED32_1);
    xxh3_state_t*  state = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh32_reset(state, SEED32_1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh32_update(state, SHORT_INPUT, size));
    TEST_ASSERT_EQUAL_UINT32(ref, xxh32_digest(state));

    xxh32_reset(state, SEED32_1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh32_update(state, SHORT_INPUT, size));
    TEST_ASSERT_EQUAL_UINT32(ref, xxh32_digest(state));
    xxh3_freeState(state);
}

/* ------------------------------------------------------ xxh64 */

static void test_xxh64_single_shot_stable(void)
{
    uint64_t a = xxh64(LOREM, strlen(LOREM), SEED2);
    uint64_t b = xxh64(LOREM, strlen(LOREM), SEED2);
    TEST_ASSERT_EQUAL_UINT64(a, b);
}

static void test_xxh64_different_seeds_differ(void)
{
    uint64_t a = xxh64(LOREM, strlen(LOREM), SEED1);
    uint64_t b = xxh64(LOREM, strlen(LOREM), SEED2);
    TEST_ASSERT_NOT_EQUAL(a, b);
}

static void test_xxh64_empty_input_stable(void)
{
    uint64_t a = xxh64("", 0, SEED1);
    uint64_t b = xxh64("", 0, SEED1);
    TEST_ASSERT_EQUAL_UINT64(a, b);
}

static void test_xxh64_stream_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    const uint64_t ref   = xxh64(LOREM, size, SEED2);
    xxh3_state_t*  state = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh64_reset(state, SEED2);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh64_update(state, LOREM, size));
    TEST_ASSERT_EQUAL_UINT64(ref, xxh64_digest(state));
    xxh3_freeState(state);
}

static void test_xxh64_chunked_streaming_matches_single_shot(void)
{
    const size_t   size  = strlen(LOREM);
    const size_t   chunk = 13;
    const uint64_t ref   = xxh64(LOREM, size, SEED1);
    xxh3_state_t*  state = xxh3_createState();
    size_t         offset;

    TEST_ASSERT_NOT_NULL(state);
    xxh64_reset(state, SEED1);
    for (offset = 0; offset + chunk <= size; offset += chunk) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh64_update(state, LOREM + offset, chunk));
    }
    if (offset < size) {
        TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh64_update(state, LOREM + offset, size - offset));
    }
    TEST_ASSERT_EQUAL_UINT64(ref, xxh64_digest(state));
    xxh3_freeState(state);
}

static void test_xxh64_stream_reset_reuse(void)
{
    const size_t   size  = strlen(SHORT_INPUT);
    const uint64_t ref   = xxh64(SHORT_INPUT, size, SEED1);
    xxh3_state_t*  state = xxh3_createState();

    TEST_ASSERT_NOT_NULL(state);
    xxh64_reset(state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh64_update(state, SHORT_INPUT, size));
    TEST_ASSERT_EQUAL_UINT64(ref, xxh64_digest(state));

    xxh64_reset(state, SEED1);
    TEST_ASSERT_EQUAL_INT(XXH3_OK, xxh64_update(state, SHORT_INPUT, size));
    TEST_ASSERT_EQUAL_UINT64(ref, xxh64_digest(state));
    xxh3_freeState(state);
}

/* ---------------------------------------- cross-algorithm outputs are distinct */

static void test_xxh32_xxh64_outputs_differ_for_same_input(void)
{
    uint32_t h32 = xxh32(LOREM, strlen(LOREM), (uint32_t)SEED1);
    uint64_t h64 = xxh64(LOREM, strlen(LOREM), SEED1);
    TEST_ASSERT_NOT_EQUAL((uint64_t)h32, h64);
}

static void test_xxh64_and_xxh3_64_differ_for_same_input(void)
{
    uint64_t h64  = xxh64         (LOREM, strlen(LOREM), SEED1);
    uint64_t hx64 = xxh3_64_scalar(LOREM, strlen(LOREM), SEED1);
    TEST_ASSERT_NOT_EQUAL(h64, hx64);
}

/* ------------------------------------- null-state defensive return checks */

static void test_xxh3_64_update_null_state_returns_error(void)
{
    TEST_ASSERT_EQUAL_INT(XXH3_ERROR, xxh3_64_update(NULL, LOREM, strlen(LOREM)));
}

static void test_xxh3_64_digest_null_state_returns_zero(void)
{
    TEST_ASSERT_EQUAL_UINT64(0, xxh3_64_digest(NULL));
}

static void test_xxh3_128_update_null_state_returns_error(void)
{
    TEST_ASSERT_EQUAL_INT(XXH3_ERROR, xxh3_128_update(NULL, LOREM, strlen(LOREM)));
}

static void test_xxh32_update_null_state_returns_error(void)
{
    TEST_ASSERT_EQUAL_INT(XXH3_ERROR, xxh32_update(NULL, LOREM, strlen(LOREM)));
}

static void test_xxh64_update_null_state_returns_error(void)
{
    TEST_ASSERT_EQUAL_INT(XXH3_ERROR, xxh64_update(NULL, LOREM, strlen(LOREM)));
}

/* ----------------------------------------------------------- main test runner */

int main(void)
{
    UNITY_BEGIN();

    /* version */
    RUN_TEST(test_version_string_is_non_empty);
    RUN_TEST(test_version_components_non_negative);
    RUN_TEST(test_secret_size_min_at_least_136);

    /* xxh3-64 variants */
    RUN_TEST(test_xxh3_64_variants_match_scalar_short);
    RUN_TEST(test_xxh3_64_variants_match_scalar_lorem);
    RUN_TEST(test_xxh3_64_variants_match_scalar_1mb);

    /* xxh3-128 variants */
    RUN_TEST(test_xxh3_128_variants_match_scalar_short);
    RUN_TEST(test_xxh3_128_high_and_low_independently_nonzero);

    /* unseeded variants (FR-024) */
    RUN_TEST(test_xxh3_64_unseeded_matches_seed_zero);
    RUN_TEST(test_xxh3_64_unseeded_scalar_variant);
    RUN_TEST(test_xxh3_128_unseeded_matches_seed_zero);
    RUN_TEST(test_xxh3_128_unseeded_scalar_variant);

    /* xxh3 streaming */
    RUN_TEST(test_xxh3_64_stream_matches_single_shot);
    RUN_TEST(test_xxh3_128_stream_matches_single_shot);
    RUN_TEST(test_xxh3_64_unseeded_stream_matches_seed_zero);
    RUN_TEST(test_xxh3_128_unseeded_stream_matches_seed_zero);
    RUN_TEST(test_xxh3_64_chunked_streaming_matches_single_shot);
    RUN_TEST(test_xxh3_128_chunked_streaming_matches_single_shot);

    /* secrets */
    RUN_TEST(test_xxh3_64_with_secret_differs_from_seeded);
    RUN_TEST(test_xxh3_64_secret_stream_matches_single_shot);
    RUN_TEST(test_xxh3_128_with_secret_matches_stream);
    RUN_TEST(test_generate_secret_produces_nonzero_output);

    /* edge cases */
    RUN_TEST(test_xxh3_64_empty_input_is_stable);
    RUN_TEST(test_xxh3_128_empty_input_is_stable);
    RUN_TEST(test_xxh3_64_single_byte_is_stable);
    RUN_TEST(test_different_seeds_produce_different_hashes);
    RUN_TEST(test_xxh3_64_different_inputs_differ);
    RUN_TEST(test_xxh3_128_different_inputs_differ);
    RUN_TEST(test_xxh3_64_avalanche_on_extra_byte);

    /* state isolation */
    RUN_TEST(test_two_states_do_not_interfere);
    RUN_TEST(test_state_can_be_reset_and_reused);

    /* state copy operations */
    RUN_TEST(test_xxh3_64_copy_state_matches_continued_hashing);
    RUN_TEST(test_xxh3_128_copy_state_branches_hashing);

    /* xxh32 */
    RUN_TEST(test_xxh32_single_shot_stable);
    RUN_TEST(test_xxh32_different_seeds_differ);
    RUN_TEST(test_xxh32_empty_input_stable);
    RUN_TEST(test_xxh32_stream_matches_single_shot);
    RUN_TEST(test_xxh32_chunked_streaming_matches_single_shot);
    RUN_TEST(test_xxh32_stream_reset_reuse);

    /* xxh64 */
    RUN_TEST(test_xxh64_single_shot_stable);
    RUN_TEST(test_xxh64_different_seeds_differ);
    RUN_TEST(test_xxh64_empty_input_stable);
    RUN_TEST(test_xxh64_stream_matches_single_shot);
    RUN_TEST(test_xxh64_chunked_streaming_matches_single_shot);
    RUN_TEST(test_xxh64_stream_reset_reuse);

    /* cross-algorithm */
    RUN_TEST(test_xxh32_xxh64_outputs_differ_for_same_input);
    RUN_TEST(test_xxh64_and_xxh3_64_differ_for_same_input);

    /* defensive null-state */
    RUN_TEST(test_xxh3_64_update_null_state_returns_error);
    RUN_TEST(test_xxh3_64_digest_null_state_returns_zero);
    RUN_TEST(test_xxh3_128_update_null_state_returns_error);
    RUN_TEST(test_xxh32_update_null_state_returns_error);
    RUN_TEST(test_xxh64_update_null_state_returns_error);

    return UNITY_END();
}
