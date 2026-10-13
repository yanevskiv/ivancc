// (Test) Status: 0
// A test file for <stdio.h> scanf's integers that fail or store nothing.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.2p10] An item that is no integer should be a matching failure.
// [C99/S7.19.6.2p16] Input ending before any conversion should return EOF.
static void Test_StdioScanIntegerFailure(void)
{
    int n = NONE;
    int first = NONE;
    int second = NONE;

    _TEST_TRUE(scans_int("", "%d", EOF, 0));
    _TEST_TRUE(scans_int("  \n", "%d", EOF, 0));
    _TEST_TRUE(scans_int("", "%i", EOF, 0));
    _TEST_TRUE(scans_int("x", "%d", 0, 0));
    _TEST_TRUE(scans_int("-", "%d", 0, 0));
    _TEST_TRUE(scans_int("+x", "%d", 0, 0));
    _TEST_TRUE(scans_int("-5", "%1d", 0, 0));
    _TEST_TRUE(scans_int("- 5", "%i", 0, 0));
    _TEST_TRUE(scans_unsigned("", "%u", EOF, 0));
    _TEST_TRUE(scans_unsigned(" ", "%x", EOF, 0));
    _TEST_TRUE(scans_unsigned("", "%o", EOF, 0));
    _TEST_TRUE(scans_unsigned("-", "%u", 0, 0));
    _TEST_TRUE(scans_unsigned("+", "%x", 0, 0));
    _TEST_TRUE(scans_unsigned("-9", "%o", 0, 0));
    _TEST_INT(sscanf("5 x", "%d%d", &first, &second), 1);
    _TEST_INT(first, 5);
    _TEST_INT(second, NONE);
    _TEST_INT(sscanf("12 34", "%*d%d%n", &first, &n), 1);
    _TEST_INT(first, 34);
    _TEST_INT(n, 5);
    _TEST_INT(sscanf("12 0x1f 7 9", "%*i%*x%*o%*u%n", &n), 0);
    _TEST_INT(n, 11);
    _TEST_INT(sscanf("123456", "%*3d%d", &first), 1);
    _TEST_INT(first, 456);
}

int main(void)
{
    Test_StdioScanIntegerFailure();
    return _Test_Status();
}
