// (Test) Status: 0
// A test file for <stdio.h> %g and %G.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %g and %G should take %f's style or %e's by the exponent.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static void Test_StdioGeneral(void)
{
    _TEST_TRUE(prints("0", "%g", 0.0));
    _TEST_TRUE(prints("-0", "%g", -0.0));
    _TEST_TRUE(prints("1", "%g", 1.0));
    _TEST_TRUE(prints("-1", "%g", -1.0));
    _TEST_TRUE(prints("1.5", "%g", 1.5));
    _TEST_TRUE(prints("0.5", "%g", 0.5));
    _TEST_TRUE(prints("100", "%g", 100.0));
    _TEST_TRUE(prints("3.14159", "%g", 3.141592653589793));
    _TEST_TRUE(prints("123456", "%g", 123456.0));

    _TEST_TRUE(prints("100000", "%g", 100000.0));
    _TEST_TRUE(prints("1e+06", "%g", 1000000.0));
    _TEST_TRUE(prints("1.23457e+06", "%g", 1234567.0));
    _TEST_TRUE(prints("0.0001", "%g", 0.0001));
    _TEST_TRUE(prints("1e-05", "%g", 0.00001));
    _TEST_TRUE(prints("0.000123457", "%g", 0.000123456789));
    _TEST_TRUE(prints("1.23457e-05", "%g", 0.0000123456789));
    _TEST_TRUE(prints("1e+100", "%g", 1e100));
    _TEST_TRUE(prints("1e-100", "%g", 1e-100));

    _TEST_TRUE(prints("1E+06", "%G", 1000000.0));
    _TEST_TRUE(prints("1E-05", "%G", 0.00001));
    _TEST_TRUE(prints("1.5", "%G", 1.5));

    _TEST_TRUE(prints("1.23e+03", "%.3g", 1234.0));
    _TEST_TRUE(prints("1234", "%.4g", 1234.0));
    _TEST_TRUE(prints("1234", "%.10g", 1234.0));
    _TEST_TRUE(prints("100", "%.3g", 100.0));
    _TEST_TRUE(prints("1e+02", "%.2g", 100.0));
    _TEST_TRUE(prints("0.00012", "%.2g", 0.000123));
    _TEST_TRUE(prints("0.1", "%.10g", 0.1));
    _TEST_TRUE(prints("3.141592653589793", "%.16g", 3.141592653589793));
    _TEST_TRUE(prints("0.10000000000000001", "%.17g", 0.1));

    _TEST_TRUE(prints("1e+02", "%.0g", 123.0));
    _TEST_TRUE(prints("1e+02", "%.g", 123.0));
    _TEST_TRUE(prints("1e+02", "%.1g", 123.0));
    _TEST_TRUE(prints("0.5", "%.0g", 0.5));
    _TEST_TRUE(prints("5", "%.0g", 5.0));
    _TEST_TRUE(prints("0", "%.0g", 0.0));

    _TEST_TRUE(prints("1.2e+03", "%.*g", 2, 1234.0));
    _TEST_TRUE(prints("1234", "%.*g", -1, 1234.0));
}

int main(void)
{
    Test_StdioGeneral();
    return _Test_Status();
}
