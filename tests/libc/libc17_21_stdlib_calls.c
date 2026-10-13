// (Test) Status: 0
// A test file for <stdlib.h> calls by pointer and past a macro.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.1.4p1] A pointer and a call in parentheses should reach each function.
static void Test_StdlibCalls(void)
{
    unsigned char *ptr = (malloc)(1);
    void *(*zeroed)(size_t, size_t) = calloc;
    long (*convert)(const char *restrict, char **restrict, int) = strtol;

    ptr = (realloc)(ptr, 2);
    _TEST_TRUE(ptr != NULL);
    (free)(ptr);
    ptr = (calloc)(1, 1);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(*ptr, 0);
        free(ptr);
    }
    ptr = zeroed(1, 1);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(*ptr, 0);
        free(ptr);
    }
    _TEST_INT(convert("-12", NULL, 10), -12);

    _TEST_FLOAT((atof)("-1"), -1.0);
    _TEST_INT((atoi)("3"), 3);
    _TEST_INT((atol)("3"), 3);
    _TEST_INT((atoll)("3"), 3);
    _TEST_FLOAT((strtod)("4", NULL), 4.0);
    _TEST_FLOAT((strtof)("4", NULL), 4.0F);
    _TEST_FLOAT((strtold)("4", NULL), 4.0L);
    _TEST_INT((strtol)("7", NULL, 10), 7);
    _TEST_INT((strtoll)("7", NULL, 10), 7);
    _TEST_UINT((strtoul)("7", NULL, 10), 7);
    _TEST_UINT((strtoull)("7", NULL, 10), 7);
    _TEST_INT((abs)(5), 5);
    _TEST_INT((labs)(-5L), 5);
    _TEST_INT((llabs)(-5LL), 5);
    _TEST_INT((div)(7, 2).quot, 3);
    _TEST_INT((ldiv)(7L, 2L).rem, 1);
    _TEST_INT((lldiv)(7LL, 2LL).quot, 3);
}

int main(void)
{
    Test_StdlibCalls();
    return _Test_Status();
}
