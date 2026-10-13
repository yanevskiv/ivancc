// (Test) Status: 0
// A test file for <stdio.h> scanf's real numbers that fail or store nothing.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.2p10] An item that is no number should be a matching failure.
// [C99/S7.19.6.2p16] Input ending before any conversion should return EOF.
static void Test_StdioScanRealFailure(void)
{
    int n = NONE;
    float first = NONE;
    float second = NONE;

    _TEST_TRUE(scans_float("", "%f", EOF, 0.0f));
    _TEST_TRUE(scans_float("  \n", "%f", EOF, 0.0f));
    _TEST_TRUE(scans_double("", "%lg", EOF, 0.0));
    _TEST_TRUE(scans_long_double(" ", "%La", EOF, 0.0L));
    _TEST_TRUE(scans_float("x", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("+", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("-x", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float(".", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("-.", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("..5", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float(".e5", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("e5", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("- 5", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("-5", "%1f", 0, 0.0f));
    _TEST_TRUE(scans_float("0x", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("0xp1", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("in", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("infin", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("infinity", "%5f", 0, 0.0f));
    _TEST_TRUE(scans_float("na", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("nan(", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("nan(ab", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("nan(a-b)", "%f", 0, 0.0f));
    _TEST_TRUE(scans_float("nan(1)", "%4f", 0, 0.0f));
    _TEST_INT(sscanf("1.5 x", "%f%f", &first, &second), 1);
    _TEST_FLOAT(first, 1.5f);
    _TEST_FLOAT(second, NONE);
    _TEST_INT(sscanf("1.5 2.5", "%*f%f%n", &first, &n), 1);
    _TEST_FLOAT(first, 2.5f);
    _TEST_INT(n, 7);
    _TEST_INT(sscanf("1e2 0x1p3 inf nan(1)", "%*e%*a%*g%*F%n", &n), 0);
    _TEST_INT(n, 20);
    _TEST_INT(sscanf("1.25.5", "%*4f%f", &first), 1);
    _TEST_FLOAT(first, 0.5f);
}

int main(void)
{
    Test_StdioScanRealFailure();
    return _Test_Status();
}
