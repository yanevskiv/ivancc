// (Test) Status: 0
// A test file for <stdio.h> the i conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %i should read an integer in the base its prefix names.
// [C99:7.19.6.2p12]: scanf's conversion specifiers
static void Test_StdioScanInteger(void)
{
    char ch = FILL;
    int value = NONE;

    _TEST_TRUE(scans_int("123", "%i", 1, 123));
    _TEST_TRUE(scans_int("-45", "%i", 1, -45));
    _TEST_TRUE(scans_int("0", "%i", 1, 0));
    _TEST_TRUE(scans_int("0123", "%i", 1, 83));
    _TEST_TRUE(scans_int("+010", "%i", 1, 8));
    _TEST_TRUE(scans_int("-017", "%i", 1, -15));
    _TEST_TRUE(scans_int("0x1f", "%i", 1, 31));
    _TEST_TRUE(scans_int("0X1F", "%i", 1, 31));
    _TEST_TRUE(scans_int("-0x10", "%i", 1, -16));
    _TEST_TRUE(scans_int("  0x7fffffff", "%i", 1, INT_MAX));
    _TEST_TRUE(scans_int("12a", "%i", 1, 12));
    _TEST_TRUE(scans_int("1f", "%i", 1, 1));
    _TEST_TRUE(scans_int("0x1fg", "%i", 1, 31));
    _TEST_TRUE(scans_int("0x1f", "%3i", 1, 1));
    _TEST_TRUE(scans_int("0123", "%3i", 1, 10));
    _TEST_TRUE(scans_int("0x1f", "%1i", 1, 0));
    _TEST_INT(sscanf("08", "%i%c", &value, &ch), 2);
    _TEST_INT(value, 0);
    _TEST_INT(ch, '8');
    _TEST_INT(sscanf("0b1", "%i%c", &value, &ch), 2);
    _TEST_INT(value, 0);
    _TEST_INT(ch, 'b');
}

int main(void)
{
    Test_StdioScanInteger();
    return _Test_Status();
}
