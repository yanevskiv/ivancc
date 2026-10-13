// (Test) Status: 0
// A test file for <stdlib.h> strtod rounding.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99 S7.20.1.3p5] strtod should round to the nearest value, a tie to even.
static void Test_StdlibStrtodRounding(void)
{
    _TEST_TRUE(reads("0.1", 0.1, 3));
    _TEST_TRUE(reads("2.2250738585072014e-308", DBL_MIN, 23));
    _TEST_TRUE(reads("1.7976931348623157e308", DBL_MAX, 22));
    _TEST_TRUE(reads("9007199254740993", 9007199254740992.0, 16));
    _TEST_TRUE(reads("9007199254740995", 9007199254740996.0, 16));
    _TEST_TRUE(reads("123456789012345678901", 123456789012345678901.0, 21));
    _TEST_TRUE(reads("2.4703282292062328e-324", DBL_LEAST, 23));

    _TEST_TRUE(reads("0x1.00000000000008p0", 1.0, 20));
    _TEST_TRUE(reads("0x1.00000000000018p0", 0x1.0000000000002p0, 20));
    _TEST_TRUE(reads("0x1.000000000000081p0", 0x1.0000000000001p0, 21));
    _TEST_TRUE(reads("0x1.fffffffffffff8p0", 2.0, 20));
}

int main(void)
{
    Test_StdlibStrtodRounding();
    return _Test_Status();
}
