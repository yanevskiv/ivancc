// (Test) Status: 0
// A test file for <stdio.h> the d conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define ZEROS "0000000000000000000000000000000000000000000000000000000000000000"

// [C99:7.19.6.2p12] %d should read a decimal integer, signed or not, as strtol.
static void Test_StdioScanDecimal(void)
{
    int first = NONE;
    int second = NONE;

    _TEST_TRUE(scans_int("123", "%d", 1, 123));
    _TEST_TRUE(scans_int("-45", "%d", 1, -45));
    _TEST_TRUE(scans_int("+7", "%d", 1, 7));
    _TEST_TRUE(scans_int("0", "%d", 1, 0));
    _TEST_TRUE(scans_int("-0", "%d", 1, 0));
    _TEST_TRUE(scans_int(" \t\n 8", "%d", 1, 8));
    _TEST_TRUE(scans_int("12ab", "%d", 1, 12));
    _TEST_TRUE(scans_int("007", "%d", 1, 7));
    _TEST_TRUE(scans_int("010", "%d", 1, 10));
    _TEST_TRUE(scans_int("0x10", "%d", 1, 0));
    _TEST_TRUE(scans_int("2147483647", "%d", 1, INT_MAX));
    _TEST_TRUE(scans_int("-2147483648", "%d", 1, INT_MIN));
    _TEST_TRUE(scans_int(ZEROS "5", "%d", 1, 5));
    _TEST_TRUE(scans_int(ZEROS ZEROS "-", "%d", 1, 0));
    _TEST_TRUE(scans_int("12345", "%3d", 1, 123));
    _TEST_TRUE(scans_int("-12", "%2d", 1, -1));
    _TEST_TRUE(scans_int("12", "%9d", 1, 12));
    _TEST_INT(sscanf("1 -2", "%d%d", &first, &second), 2);
    _TEST_INT(first, 1);
    _TEST_INT(second, -2);
    _TEST_INT(sscanf("3-4", "%d%d", &first, &second), 2);
    _TEST_INT(first, 3);
    _TEST_INT(second, -4);
    _TEST_INT(sscanf("1234", "%2d%2d", &first, &second), 2);
    _TEST_INT(first, 12);
    _TEST_INT(second, 34);
}

int main(void)
{
    Test_StdioScanDecimal();
    return _Test_Status();
}
