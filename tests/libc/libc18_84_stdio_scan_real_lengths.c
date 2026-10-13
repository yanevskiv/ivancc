// (Test) Status: 0
// A test file for <stdio.h> scanf's floating conversions of each length.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.2p11] l should store a double, L a long double and no length a
// float, each the value strtod, strtold and strtof give.
static void Test_StdioScanRealLengths(void)
{
    double pair[2] = {NONE, NONE};
    const char *format[] = {"%lf", "%le", "%lg", "%la", "%lF", "%lE", "%lG", "%lA"};
    const char *extended[] = {"%Lf", "%Le", "%Lg", "%La", "%LF", "%LE", "%LG", "%LA"};

    _TEST_TRUE(scans_float("0.1", "%f", 1, strtof("0.1", NULL)));
    _TEST_TRUE(scans_double("0.1", "%lf", 1, strtod("0.1", NULL)));
    _TEST_TRUE(scans_long_double("0.1", "%Lf", 1, strtold("0.1", NULL)));
    _TEST_TRUE(scans_float("16777217", "%f", 1, 16777216.0f));
    _TEST_TRUE(scans_double("16777217", "%lf", 1, 16777217.0));
    _TEST_TRUE(scans_double("9007199254740993", "%lf", 1, 9007199254740992.0));
    _TEST_TRUE(scans_long_double("9007199254740993", "%Lf", 1, 9007199254740993.0L));
    _TEST_TRUE(scans_float("3.4028234e38", "%f", 1, FLT_MAX));
    _TEST_TRUE(scans_float("1.17549435e-38", "%f", 1, FLT_MIN));
    _TEST_TRUE(scans_double("1.7976931348623157e308", "%lf", 1, DBL_MAX));
    _TEST_TRUE(scans_double("2.2250738585072014e-308", "%lf", 1, DBL_MIN));
    _TEST_TRUE(scans_double("-1e300", "%lf", 1, -1e300));
    _TEST_TRUE(scans_long_double("1e4000", "%Lf", 1, 1e4000L));
    _TEST_TRUE(scans_long_double("-1e-4000", "%Lf", 1, -1e-4000L));
    _TEST_TRUE(scans_long_double("0x1.fffffffffffffffep0", "%Lf", 1, 0x1.fffffffffffffffep0L));
    for (size_t i = 0; i < sizeof(format) / sizeof(format[0]); i++) {
        _TEST_TRUE(scans_double("-1.5e2", format[i], 1, -150.0));
        _TEST_TRUE(scans_double("0x1p-1074", format[i], 1, 0x1p-1074));
        _TEST_TRUE(scans_long_double("-1.5e2", extended[i], 1, -150.0L));
        _TEST_TRUE(scans_long_double("0x1p-16445", extended[i], 1, 0x1p-16445L));
    }

    // A double stored should leave what follows it as it was.
    _TEST_INT(sscanf("2.5", "%lf", &pair[0]), 1);
    _TEST_FLOAT(pair[0], 2.5);
    _TEST_FLOAT(pair[1], NONE);
}

int main(void)
{
    Test_StdioScanRealLengths();
    return _Test_Status();
}
