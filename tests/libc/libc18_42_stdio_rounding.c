// (Test) Status: 0
// A test file for <stdio.h> rounding of floating conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.1p13] A floating conversion should round its value correctly.
static void Test_StdioRounding(void)
{
    _TEST_TRUE(prints("0", "%.0f", 0.5));
    _TEST_TRUE(prints("2", "%.0f", 1.5));
    _TEST_TRUE(prints("2", "%.0f", 2.5));
    _TEST_TRUE(prints("4", "%.0f", 3.5));
    _TEST_TRUE(prints("-2", "%.0f", -2.5));
    _TEST_TRUE(prints("0.2", "%.1f", 0.25));
    _TEST_TRUE(prints("0.8", "%.1f", 0.75));
    _TEST_TRUE(prints("0.12", "%.2f", 0.125));
    _TEST_TRUE(prints("0.38", "%.2f", 0.375));
    _TEST_TRUE(prints("2e+00", "%.0e", 2.5));
    _TEST_TRUE(prints("4e+00", "%.0e", 3.5));
    _TEST_TRUE(prints("1.2e+00", "%.1e", 1.25));
    _TEST_TRUE(prints("1.8e+00", "%.1e", 1.75));
    _TEST_TRUE(prints("2", "%.1g", 2.5));
    _TEST_TRUE(prints("0.12", "%.2g", 0.125));

    _TEST_TRUE(prints("0", "%.0f", 0.4));
    _TEST_TRUE(prints("1", "%.0f", 0.6));
    _TEST_TRUE(prints("-0", "%.0f", -0.4));
    _TEST_TRUE(prints("-1", "%.0f", -0.6));
    _TEST_TRUE(prints("0.0", "%.1f", 0.04));
    _TEST_TRUE(prints("0.1", "%.1f", 0.06));
    _TEST_TRUE(prints("0.000", "%.3f", 0.0004));
    _TEST_TRUE(prints("0.001", "%.3f", 0.0006));

    _TEST_TRUE(prints("0.1", "%.1f", 0.05));
    _TEST_TRUE(prints("0.1", "%.1f", 0.15));
    _TEST_TRUE(prints("0.3", "%.1f", 0.35));
    _TEST_TRUE(prints("1.00", "%.2f", 1.005));
    _TEST_TRUE(prints("2.67", "%.2f", 2.675));
    _TEST_TRUE(prints("1.0e-01", "%.1e", 0.105));

    _TEST_TRUE(prints("10", "%.0f", 9.5));
    _TEST_TRUE(prints("100", "%.0f", 99.5));
    _TEST_TRUE(prints("10.00", "%.2f", 9.999));
    _TEST_TRUE(prints("1.00", "%.2f", 0.999));
    _TEST_TRUE(prints("1e+01", "%.0e", 9.5));
    _TEST_TRUE(prints("1.0e+01", "%.1e", 9.96));
    _TEST_TRUE(prints("1.00e+03", "%.2e", 999.9));
    _TEST_TRUE(prints("1e+03", "%.3g", 999.9));
    _TEST_TRUE(prints("10", "%.2g", 9.96));
    _TEST_TRUE(prints("1e+01", "%.1g", 9.5));
    _TEST_TRUE(prints("0.001", "%.2g", 0.000999));

    _TEST_TRUE(prints("0.29999999999999998890", "%.20f", 0.3));
    _TEST_TRUE(prints("1.00000000000000005551e-01", "%.20e", 0.1));
    _TEST_TRUE(prints("0.1000000000000000055511151231257827021181583404541015625", "%.55f", 0.1));
    _TEST_TRUE(prints("0.1000000000000000055511151231257827021181583404541015625", "%.60g", 0.1));
}

int main(void)
{
    Test_StdioRounding();
    return _Test_Status();
}
