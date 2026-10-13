// (Test) Status: 0
// A test file for <stdlib.h> strtod forms of a number.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.20.1.3p3] strtod should read each decimal and hexadecimal form alone.
static void Test_StdlibStrtodForms(void)
{
    _TEST_TRUE(reads("1.5", 1.5, 3));
    _TEST_TRUE(reads("  -2.25e1", -22.5, 9));
    _TEST_TRUE(reads("+.5", 0.5, 3));
    _TEST_TRUE(reads("5.", 5.0, 2));
    _TEST_TRUE(reads("12abc", 12.0, 2));
    _TEST_TRUE(reads("\t\n\v\f\r 7", 7.0, 7));
    _TEST_TRUE(reads("1.5.5", 1.5, 3));
    _TEST_TRUE(reads("0x1.8.8", 1.5, 5));
    _TEST_TRUE(reads("1e", 1.0, 1));
    _TEST_TRUE(reads("1e+", 1.0, 1));
    _TEST_TRUE(reads("1e+x", 1.0, 1));
    _TEST_TRUE(reads("1.5E+00002", 150.0, 10));
    _TEST_TRUE(reads("0x", 0.0, 1));
    _TEST_TRUE(reads("0x1.8p1", 3.0, 7));
    _TEST_TRUE(reads("0X1P-2", 0.25, 6));
    _TEST_TRUE(reads("0x1p", 1.0, 3));
    _TEST_TRUE(reads("0x.8", 0.5, 4));
    _TEST_TRUE(reads("-0xAbp0", -171.0, 7));

    _TEST_TRUE(reads("", 0.0, 0));
    _TEST_TRUE(reads(".", 0.0, 0));
    _TEST_TRUE(reads("-", 0.0, 0));
    _TEST_TRUE(reads("e5", 0.0, 0));
    _TEST_TRUE(reads(".e1", 0.0, 0));
    _TEST_TRUE(reads("x", 0.0, 0));
}

int main(void)
{
    Test_StdlibStrtodForms();
    return _Test_Status();
}
