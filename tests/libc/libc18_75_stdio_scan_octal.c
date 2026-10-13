// (Test) Status: 0
// A test file for <stdio.h> the o conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.2p12] %o should read an octal integer as strtoul does in base 8.
static void Test_StdioScanOctal(void)
{
    char ch = FILL;
    unsigned int value = FILL;

    _TEST_TRUE(scans_unsigned("17", "%o", 1, 15));
    _TEST_TRUE(scans_unsigned("017", "%o", 1, 15));
    _TEST_TRUE(scans_unsigned("+17", "%o", 1, 15));
    _TEST_TRUE(scans_unsigned("0", "%o", 1, 0));
    _TEST_TRUE(scans_unsigned(" \n7", "%o", 1, 7));
    _TEST_TRUE(scans_unsigned("37777777777", "%o", 1, UINT_MAX));
    _TEST_TRUE(scans_unsigned("777", "%2o", 1, 63));
    _TEST_TRUE(scans_unsigned("8", "%o", 0, 0));
    _TEST_INT(sscanf("178", "%o%c", &value, &ch), 2);
    _TEST_UINT(value, 15);
    _TEST_INT(ch, '8');
    _TEST_INT(sscanf("0x7", "%o%c", &value, &ch), 2);
    _TEST_UINT(value, 0);
    _TEST_INT(ch, 'x');
}

int main(void)
{
    Test_StdioScanOctal();
    return _Test_Status();
}
