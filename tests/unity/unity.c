#include "unity.h"

int UnityFailCount = 0;
int UnityRunCount = 0;

void UnityBegin(const char* file)
{
    printf("UNITY BEGIN: %s\n", file);
    UnityFailCount = 0;
    UnityRunCount = 0;
}

int UnityEnd(void)
{
    printf("UNITY END: %d run, %d failed\n", UnityRunCount, UnityFailCount);
    return UnityFailCount;
}

void UnityDefaultTestRun(UnityTestFunction test, const char* testName, int lineNumber)
{
    (void)lineNumber;
    UnityRunCount++;
    setUp();
    test();
    tearDown();
    if (UnityFailCount == 0) {
        printf("PASS: %s\n", testName);
    }
}

void UnityFail(const char* message, const char* file, int lineNumber)
{
    fprintf(stderr, "FAIL: %s (%s:%d)\n", message, file, lineNumber);
    UnityFailCount++;
}
