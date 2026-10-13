// (Test) Status: 0
// A test file for <stdlib.h> strtod zeros and limits.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99/S7.20.1.3p4] strtod should read either zero and the least and greatest.
static void Test_StdlibStrtodLimits(void)
{
    _TEST_TRUE(reads("0", 0.0, 1));
    _TEST_TRUE(reads("-0", -nought, 2));
    _TEST_TRUE(reads("-0x0p3", -nought, 6));
    _TEST_TRUE(reads("0x1p-1022", DBL_MIN, 9));
    _TEST_TRUE(reads("0x1.fffffffffffffp1023", DBL_MAX, 22));
    _TEST_TRUE(reads("0x1p-1074", DBL_LEAST, 9));
    _TEST_TRUE(reads("0x0.0000000000001p-1022", DBL_LEAST, 23));
    _TEST_TRUE(reads("1e0000000000000000000000000000000001", 10.0, 36));
    _TEST_TRUE(reads("0e999999999999999999999", 0.0, 23));
}

int main(void)
{
    Test_StdlibStrtodLimits();
    return _Test_Status();
}
