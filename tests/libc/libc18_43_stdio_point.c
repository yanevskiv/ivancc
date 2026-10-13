// (Test) Status: 0
// A test file for <stdio.h> the # flag on floating conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.1p6] A # flag should keep a decimal point, and %g's trailing zeros.
static void Test_StdioPoint(void)
{
    _TEST_TRUE(prints("3", "%.0f", 3.0));
    _TEST_TRUE(prints("3.", "%#.0f", 3.0));
    _TEST_TRUE(prints("-3.", "%#.0f", -3.0));
    _TEST_TRUE(prints("0.", "%#.0f", 0.0));
    _TEST_TRUE(prints("3.", "%#.0F", 3.0));
    _TEST_TRUE(prints("1.500000", "%#f", 1.5));
    _TEST_TRUE(prints("1.5", "%#.1f", 1.5));

    _TEST_TRUE(prints("3e+00", "%.0e", 3.0));
    _TEST_TRUE(prints("3.e+00", "%#.0e", 3.0));
    _TEST_TRUE(prints("3.E+00", "%#.0E", 3.0));
    _TEST_TRUE(prints("0.e+00", "%#.0e", 0.0));
    _TEST_TRUE(prints("1.500000e+00", "%#e", 1.5));

    _TEST_TRUE(prints("1.5", "%g", 1.5));
    _TEST_TRUE(prints("1.50000", "%#g", 1.5));
    _TEST_TRUE(prints("1.00000", "%#g", 1.0));
    _TEST_TRUE(prints("0.00000", "%#g", 0.0));
    _TEST_TRUE(prints("100000.", "%#g", 100000.0));
    _TEST_TRUE(prints("1.00000e+10", "%#g", 1e10));
    _TEST_TRUE(prints("1.00000E+10", "%#G", 1e10));
    _TEST_TRUE(prints("0.000100000", "%#g", 0.0001));
    _TEST_TRUE(prints("1.00000e-05", "%#g", 0.00001));
    _TEST_TRUE(prints("1.00", "%#.3g", 1.0));
    _TEST_TRUE(prints("0.000120", "%#.3g", 0.00012));
    _TEST_TRUE(prints("1.", "%#.1g", 1.0));
    _TEST_TRUE(prints("1.", "%#.0g", 1.0));
    _TEST_TRUE(prints("0.", "%#.0g", 0.0));
    _TEST_TRUE(prints("1.e+02", "%#.1g", 123.0));

    _TEST_TRUE(prints("    3.", "%#6.0f", 3.0));
    _TEST_TRUE(prints("3.    |", "%-#6.0f|", 3.0));
    _TEST_TRUE(prints("00003.", "%#06.0f", 3.0));
}

int main(void)
{
    Test_StdioPoint();
    return _Test_Status();
}
