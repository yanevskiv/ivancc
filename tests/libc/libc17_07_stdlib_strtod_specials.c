// (Test) Status: 0
// A test file for <stdlib.h> strtod infinities and NaNs.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99/S7.20.1.3p3] strtod should read an infinity and a NaN, in any case.
static void Test_StdlibStrtodSpecials(void)
{
    double invalid = nought / nought;
    double infinite = 1 / nought;

    _TEST_TRUE(reads("inf", infinite, 3));
    _TEST_TRUE(reads("INFINITY", infinite, 8));
    _TEST_TRUE(reads("-Inf", -infinite, 4));
    _TEST_TRUE(reads("infinit", infinite, 3));

    _TEST_TRUE(reads("nan", invalid, 3));
    _TEST_TRUE(reads("-NaN", invalid, 4));
    _TEST_TRUE(reads("nan()", invalid, 5));
    _TEST_TRUE(reads("nan(123)", invalid, 8));
    _TEST_TRUE(reads("NAN(abc_9)", invalid, 10));
    _TEST_TRUE(reads("nan(", invalid, 3));
    _TEST_TRUE(reads("nan(-1)", invalid, 3));
}

int main(void)
{
    Test_StdlibStrtodSpecials();
    return _Test_Status();
}
