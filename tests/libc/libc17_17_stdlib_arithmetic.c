// (Test) Status: 0
// A test file for <stdlib.h> abs and div and their kin.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.20.6.2p2] abs should drop a sign, and div truncate toward zero.
static void Test_StdlibArithmetic(void)
{
    _TEST_INT(abs(-5), 5);
    _TEST_INT(abs(INT_MIN + 1), INT_MAX);
    _TEST_INT(labs(-5L), 5);
    _TEST_INT(labs(LONG_MIN + 1), LONG_MAX);
    _TEST_INT(llabs(-5LL), 5);
    _TEST_INT(llabs(LLONG_MIN + 1), LLONG_MAX);

    _TEST_INT(div(-7, 2).quot, -3);
    _TEST_INT(div(-7, 2).rem, -1);
    _TEST_INT(ldiv(7L, -2L).quot, -3);
    _TEST_INT(ldiv(7L, -2L).rem, 1);
    _TEST_INT(lldiv(-7LL, -2LL).quot, 3);
    _TEST_INT(lldiv(-7LL, -2LL).rem, -1);
}

int main(void)
{
    Test_StdlibArithmetic();
    return _Test_Status();
}
