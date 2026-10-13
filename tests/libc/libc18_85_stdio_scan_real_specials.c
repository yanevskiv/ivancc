// (Test) Status: 0
// A test file for <stdio.h> scanf's infinities and NaNs.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.2p12] %f should read an infinity or a NaN, as strtod reads one.
static void Test_StdioScanRealSpecials(void)
{
    char ch = FILL;
    double value = NONE;
    double invalid = strtod("nan", NULL);
    double infinite = strtod("inf", NULL);

    _TEST_TRUE(scans_double("inf", "%lf", 1, infinite));
    _TEST_TRUE(scans_double("INF", "%lf", 1, infinite));
    _TEST_TRUE(scans_double("infinity", "%lf", 1, infinite));
    _TEST_TRUE(scans_double("InFiNiTy", "%lf", 1, infinite));
    _TEST_TRUE(scans_double("-inf", "%lf", 1, -infinite));
    _TEST_TRUE(scans_double("+infinity", "%lf", 1, infinite));
    _TEST_TRUE(scans_double("  -INFINITY", "%lf", 1, -infinite));
    _TEST_TRUE(scans_double("infinity", "%3lf", 1, infinite));
    _TEST_TRUE(scans_double("nan", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("NAN", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("-nan", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("nan()", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("nan(123)", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("NaN(abc_9)", "%lf", 1, invalid));
    _TEST_TRUE(scans_double("nan(1)", "%3lf", 1, invalid));
    _TEST_TRUE(scans_float("-inf", "%f", 1, (float) -infinite));
    _TEST_TRUE(scans_float("nan", "%e", 1, (float) invalid));
    _TEST_TRUE(scans_long_double("infinity", "%Lg", 1, infinite));
    _TEST_TRUE(scans_long_double("nan(0)", "%La", 1, invalid));
    _TEST_INT(sscanf("infinityx", "%lf%c", &value, &ch), 2);
    _TEST_FLOAT(value, infinite);
    _TEST_INT(ch, 'x');
    _TEST_INT(sscanf("nanx", "%lf%c", &value, &ch), 2);
    _TEST_FLOAT(value, invalid);
    _TEST_INT(ch, 'x');
    _TEST_INT(sscanf("NAN(abc_1)z", "%lf%c", &value, &ch), 2);
    _TEST_FLOAT(value, invalid);
    _TEST_INT(ch, 'z');
}

int main(void)
{
    Test_StdioScanRealSpecials();
    return _Test_Status();
}
