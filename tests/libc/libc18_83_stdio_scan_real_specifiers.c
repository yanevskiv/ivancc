// (Test) Status: 0
// A test file for <stdio.h> scanf's a, e and g conversions and the capitals.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.2p12) %a, %e and %g should read a floating number as %f does.
// (S7.19.6.2p14) %A, %E, %F and %G should behave as %a, %e, %f and %g.
static void Test_StdioScanRealSpecifiers(void)
{
    const char *format[] = {"%a", "%e", "%f", "%g", "%A", "%E", "%F", "%G"};

    for (size_t i = 0; i < sizeof(format) / sizeof(format[0]); i++) {
        _TEST_TRUE(scans_float("1.5", format[i], 1, 1.5f));
        _TEST_TRUE(scans_float("-25e-2", format[i], 1, -0.25f));
        _TEST_TRUE(scans_float("0x1.8p1", format[i], 1, 3.0f));
        _TEST_TRUE(scans_float("0X1P-1", format[i], 1, 0.5f));
        _TEST_TRUE(scans_float(" 100", format[i], 1, 100.0f));
        _TEST_TRUE(scans_float("x", format[i], 0, 0.0f));
        _TEST_TRUE(scans_float("", format[i], EOF, 0.0f));
    }
}

int main(void)
{
    Test_StdioScanRealSpecifiers();
    return _Test_Status();
}
