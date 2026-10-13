// (Test) Status: 0
// A test file for <stdio.h> %e and %E.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.1p8) %e and %E should write one digit, a point, six and an exponent.
static void Test_StdioExponent(void)
{
    _TEST_TRUE(prints("0.000000e+00", "%e", 0.0));
    _TEST_TRUE(prints("-0.000000e+00", "%e", -0.0));
    _TEST_TRUE(prints("1.000000e+00", "%e", 1.0));
    _TEST_TRUE(prints("-1.000000e+00", "%e", -1.0));
    _TEST_TRUE(prints("1.500000e+00", "%e", 1.5));
    _TEST_TRUE(prints("1.234568e+02", "%e", 123.4567891));
    _TEST_TRUE(prints("1.000000e+01", "%e", 10.0));
    _TEST_TRUE(prints("1.000000e-01", "%e", 0.1));
    _TEST_TRUE(prints("1.000000e+10", "%e", 1e10));
    _TEST_TRUE(prints("1.000000e-10", "%e", 1e-10));
    _TEST_TRUE(prints("9.999990e+99", "%e", 9.99999e99));

    _TEST_TRUE(prints("1.000000e+100", "%e", 1e100));
    _TEST_TRUE(prints("1.000000e-100", "%e", 1e-100));
    _TEST_TRUE(prints("1.797693e+308", "%e", DBL_MAX));
    _TEST_TRUE(prints("2.225074e-308", "%e", DBL_MIN));
    _TEST_TRUE(prints("2.220446e-16", "%e", DBL_EPSILON));

    _TEST_TRUE(prints("1.500000E+00", "%E", 1.5));
    _TEST_TRUE(prints("1.000000E-10", "%E", 1e-10));
    _TEST_TRUE(prints("1.5E-05", "%.1E", 0.000015));
    _TEST_TRUE(prints("0.000000E+00", "%E", 0.0));

    _TEST_TRUE(prints("1e+04", "%.0e", 12345.0));
    _TEST_TRUE(prints("1e+04", "%.e", 12345.0));
    _TEST_TRUE(prints("1.2e+04", "%.1e", 12345.0));
    _TEST_TRUE(prints("1.23e+04", "%.2e", 12345.0));
    _TEST_TRUE(prints("1.235e-03", "%.3e", 0.00123456));
    _TEST_TRUE(prints("1.0000000000e+00", "%.10e", 1.0));
    _TEST_TRUE(prints("0e+00", "%.0e", 0.0));
    _TEST_TRUE(prints("5.00000000000000000000e-01", "%.20e", 0.5));

    _TEST_TRUE(prints("1.23e+04", "%.*e", 2, 12345.0));
    _TEST_TRUE(prints("1.234500e+04", "%.*e", -1, 12345.0));
    _TEST_TRUE(prints("[1.500000e+00]", "[%e]", 1.5));
}

int main(void)
{
    Test_StdioExponent();
    return _Test_Status();
}
