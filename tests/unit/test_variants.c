#include <stdint.h>
#include <string.h>

#include "xxh3.h"
#include "../unity/unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_variants_64_should_match_scalar(void)
{
    const char* input = "xxh3-wrapper-test";
    uint64_t seed = 0x1234567890abcdefULL;
    size_t size = strlen(input);
    uint64_t scalar = xxh3_64_scalar(input, size, seed);

    TEST_ASSERT_EQUAL_UINT64(scalar, xxh3_64_sse2(input, size, seed));
    TEST_ASSERT_EQUAL_UINT64(scalar, xxh3_64_avx2(input, size, seed));
    TEST_ASSERT_EQUAL_UINT64(scalar, xxh3_64_avx512(input, size, seed));
    TEST_ASSERT_EQUAL_UINT64(scalar, xxh3_64_neon(input, size, seed));
    TEST_ASSERT_EQUAL_UINT64(scalar, xxh3_64_sve(input, size, seed));
}

static void test_streaming_should_produce_nonzero_digest(void)
{
    const char* input = "xxh3-wrapper-test";
    uint64_t seed = 0x1234567890abcdefULL;
    xxh3_state_t* state64 = xxh3_createState();
    xxh3_state_t* state128 = xxh3_createState();
    xxh3_128_t digest128;

    TEST_ASSERT_TRUE(state64 != NULL);
    TEST_ASSERT_TRUE(state128 != NULL);

    xxh3_64_reset(state64, seed);
    xxh3_128_reset(state128, seed);
    TEST_ASSERT_TRUE(xxh3_64_update(state64, input, strlen(input)) == XXH3_OK);
    TEST_ASSERT_TRUE(xxh3_128_update(state128, input, strlen(input)) == XXH3_OK);

    (void)xxh3_64_digest(state64);
    digest128 = xxh3_128_digest(state128);
    TEST_ASSERT_TRUE(!(digest128.high == 0 && digest128.low == 0));

    xxh3_freeState(state64);
    xxh3_freeState(state128);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_variants_64_should_match_scalar);
    RUN_TEST(test_streaming_should_produce_nonzero_digest);
    return UNITY_END();
}
