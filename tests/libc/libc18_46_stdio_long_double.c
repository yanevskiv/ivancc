// (Test) Status: 0
// A test file for <stdio.h> the L and l lengths on floating conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.1p7] L should give a long double argument, and l change nothing.
static void Test_StdioLongDouble(void)
{
    _TEST_TRUE(prints("1.500000", "%Lf", 1.5L));
    _TEST_TRUE(prints("-1.500000", "%Lf", -1.5L));
    _TEST_TRUE(prints("0.000000", "%Lf", 0.0L));
    _TEST_TRUE(prints("-0.000000", "%Lf", -0.0L));
    _TEST_TRUE(prints("1.500000", "%LF", 1.5L));
    _TEST_TRUE(prints("1.500000e+00", "%Le", 1.5L));
    _TEST_TRUE(prints("1.500000E+00", "%LE", 1.5L));
    _TEST_TRUE(prints("1.5", "%Lg", 1.5L));
    _TEST_TRUE(prints("1.5", "%LG", 1.5L));

    _TEST_TRUE(prints("0.10000000000000000000", "%.20Lf", 0.1L));
    _TEST_TRUE(prints("0.1000000000000000000013553", "%.25Lf", 0.1L));
    _TEST_TRUE(prints("3.14159265358979324", "%.18Lg", 3.14159265358979323846L));
    _TEST_TRUE(prints("3.333333333333333333423683514374e-01", "%.30Le", 1.0L / 3.0L));
    _TEST_TRUE(prints("1.0000000000000000001", "%.20Lg", 1.0L + LDBL_EPSILON));
    _TEST_TRUE(prints("18446744073709551615", "%.0Lf", 18446744073709551615.0L));
    _TEST_TRUE(prints("9223372036854775807", "%.19Lg", 9223372036854775807.0L));

    _TEST_TRUE(prints("1.189731e+4932", "%Le", LDBL_MAX));
    _TEST_TRUE(prints("3.362103e-4932", "%Le", LDBL_MIN));
    _TEST_TRUE(prints("1.084202e-19", "%Le", LDBL_EPSILON));
    _TEST_TRUE(prints("1e+600", "%Lg", 1e600L));
    _TEST_TRUE(prints("1.000000E-600", "%LE", 1e-600L));

    _TEST_TRUE(prints("+1.23e+04", "%+.2Le", 12345.678L));
    _TEST_TRUE(prints("-000003.14", "%010.2Lf", -3.14159L));
    _TEST_TRUE(prints("2.", "%#.0Lf", 2.5L));
    _TEST_TRUE(prints("      1.50", "%*.*Lf", 10, 2, 1.5L));
    _TEST_TRUE(prints("1.5 2 2.5", "%Lg %d %g", 1.5L, 2, 2.5));

    _TEST_TRUE(prints("1.500000", "%lf", 1.5));
    _TEST_TRUE(prints("1.500000", "%lF", 1.5));
    _TEST_TRUE(prints("1.500000e+00", "%le", 1.5));
    _TEST_TRUE(prints("1.500000E+00", "%lE", 1.5));
    _TEST_TRUE(prints("1.5", "%lg", 1.5));
    _TEST_TRUE(prints("1.5", "%lG", 1.5));
    _TEST_TRUE(prints("  +1.50", "%+7.2lf", 1.5));
}

int main(void)
{
    Test_StdioLongDouble();
    return _Test_Status();
}
