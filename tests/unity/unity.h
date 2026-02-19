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

#endif
