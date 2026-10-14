// (Test) Status: 0
// A test file for <stdlib.h> macros.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:7.20p3] The macros should be defined, RAND_MAX at least 32767 in #if.
#define DIRECTIVE 0
#if defined(EXIT_FAILURE) \
    && defined(EXIT_SUCCESS) \
    && defined(RAND_MAX) \
    && (RAND_MAX >= 32767)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// [C99:7.20p3] The macros should be defined and within their limits.
static void Test_StdlibMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_TRUE(RAND_MAX >= 32767);
    _TEST_TRUE(EXIT_FAILURE != EXIT_SUCCESS);
    _TEST_TRUE(MB_CUR_MAX >= 1 && MB_CUR_MAX <= MB_LEN_MAX);
    _TEST_TRUE(NULL == (void *) 0);
}

int main(void)
{
    Test_StdlibMacros();
    return _Test_Status();
}
