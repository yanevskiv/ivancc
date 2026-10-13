// (Test) Status: 0
// A test file for <stdio.h> the x and X conversions of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.2p12] %x should read a hexadecimal integer, 0x before it or not.
// [C99:S7.19.6.2p14] %X should behave the same as %x.
static void Test_StdioScanHex(void)
{
    char ch = FILL;
    unsigned int value = FILL;

    _TEST_TRUE(scans_unsigned("1f", "%x", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("1F", "%x", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("0x1f", "%x", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("0X1F", "%x", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("1f", "%X", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("0x1F", "%X", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("+a", "%x", 1, 10));
    _TEST_TRUE(scans_unsigned("0", "%x", 1, 0));
    _TEST_TRUE(scans_unsigned(" \tabcdef", "%x", 1, 0xabcdef));
    _TEST_TRUE(scans_unsigned("ffffffff", "%x", 1, UINT_MAX));
    _TEST_TRUE(scans_unsigned("0xFFFFFFFF", "%X", 1, UINT_MAX));
    _TEST_TRUE(scans_unsigned("fg", "%x", 1, 15));
    _TEST_TRUE(scans_unsigned("0xfg", "%x", 1, 15));
    _TEST_TRUE(scans_unsigned("0x1f", "%3x", 1, 1));
    _TEST_TRUE(scans_unsigned("0x1f", "%4x", 1, 0x1f));
    _TEST_TRUE(scans_unsigned("1234", "%2X", 1, 0x12));
    _TEST_TRUE(scans_unsigned("g", "%x", 0, 0));
    _TEST_TRUE(scans_unsigned("x1", "%x", 0, 0));
    _TEST_INT(sscanf("00x1", "%x%c", &value, &ch), 2);
    _TEST_UINT(value, 0);
    _TEST_INT(ch, 'x');
    _TEST_INT(sscanf("0x1", "%1x%c", &value, &ch), 2);
    _TEST_UINT(value, 0);
    _TEST_INT(ch, 'x');
}

int main(void)
{
    Test_StdioScanHex();
    return _Test_Status();
}
