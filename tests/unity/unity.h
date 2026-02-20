#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>

typedef void (*UnityTestFunction)(void);

extern int UnityFailCount;
extern int UnityRunCount;

void UnityBegin(const char* file);
int UnityEnd(void);
void UnityDefaultTestRun(UnityTestFunction test, const char* testName, int lineNumber);
void UnityFail(const char* message, const char* file, int lineNumber);

void setUp(void);
void tearDown(void);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()
#define RUN_TEST(func) UnityDefaultTestRun((func), #func, __LINE__)

#define TEST_ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            UnityFail("Expected condition to be true", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

/* Compatibility: Unity helper used in tests to skip a test with a message. */
#ifndef TEST_IGNORE_MESSAGE
#  define TEST_IGNORE_MESSAGE(msg) do { (void)(msg); return; } while (0)
#endif

#define TEST_ASSERT_EQUAL_UINT64(expected, actual) \
    do { \
        unsigned long long unity_exp = (unsigned long long)(expected); \
        unsigned long long unity_act = (unsigned long long)(actual); \
        if (unity_exp != unity_act) { \
            fprintf(stderr, "Expected %llu, got %llu\n", unity_exp, unity_act); \
            UnityFail("64-bit values differ", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
    do { \
        unsigned long unity_exp = (unsigned long)(unsigned int)(expected); \
        unsigned long unity_act = (unsigned long)(unsigned int)(actual); \
        if (unity_exp != unity_act) { \
            fprintf(stderr, "Expected %lu, got %lu\n", unity_exp, unity_act); \
            UnityFail("32-bit values differ", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_UINT(expected, actual) \
    do { \
        unsigned long unity_exp = (unsigned long)(expected); \
        unsigned long unity_act = (unsigned long)(actual); \
        if (unity_exp != unity_act) { \
            fprintf(stderr, "Expected %lu, got %lu\n", unity_exp, unity_act); \
            UnityFail("unsigned values differ", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        long unity_exp = (long)(expected); \
        long unity_act = (long)(actual); \
        if (unity_exp != unity_act) { \
            fprintf(stderr, "Expected %ld, got %ld\n", unity_exp, unity_act); \
            UnityFail("int values differ", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            UnityFail("Expected condition to be false", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            UnityFail("Expected non-NULL pointer", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            UnityFail("Expected NULL pointer", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_EQUAL(unexpected, actual) \
    do { \
        if ((unsigned long long)(unexpected) == (unsigned long long)(actual)) { \
            UnityFail("Expected values to differ", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_GREATER_OR_EQUAL_INT(threshold, actual) \
    do { \
        long unity_thr = (long)(threshold); \
        long unity_act = (long)(actual); \
        if (unity_act < unity_thr) { \
            fprintf(stderr, "Expected >= %ld, got %ld\n", unity_thr, unity_act); \
            UnityFail("value below threshold", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#endif
