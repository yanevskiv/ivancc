// (Test) Status: 0
// A test file for <stdio.h> the u conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.2p12] %u should read a decimal integer as strtoul does in base 10.
static void Test_StdioScanUnsigned(void)
{
    char ch = FILL;
    unsigned int value = FILL;

    _TEST_TRUE(scans_unsigned("123", "%u", 1, 123));
    _TEST_TRUE(scans_unsigned("+5", "%u", 1, 5));
    _TEST_TRUE(scans_unsigned("0", "%u", 1, 0));
    _TEST_TRUE(scans_unsigned("-0", "%u", 1, 0));
    _TEST_TRUE(scans_unsigned("  9", "%u", 1, 9));
    _TEST_TRUE(scans_unsigned("010", "%u", 1, 10));
    _TEST_TRUE(scans_unsigned("4294967295", "%u", 1, UINT_MAX));
    _TEST_TRUE(scans_unsigned("12345", "%3u", 1, 123));
    _TEST_TRUE(scans_unsigned("a", "%u", 0, 0));
    _TEST_INT(sscanf("12-3", "%u%c", &value, &ch), 2);
    _TEST_UINT(value, 12);
    _TEST_INT(ch, '-');
}

int main(void)
{
    Test_StdioScanUnsigned();
    return _Test_Status();
}
