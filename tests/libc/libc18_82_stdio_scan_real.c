// (Test) Status: 0
// A test file for <stdio.h> the f conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define ZEROS "0000000000000000000000000000000000000000000000000000000000000000"

// [C99:7.19.6.2p12] %f should read a floating number, signed or not, as strtod.
static void Test_StdioScanReal(void)
{
    float first = NONE;
    float second = NONE;

    _TEST_TRUE(scans_float("1.5", "%f", 1, 1.5f));
    _TEST_TRUE(scans_float("-2.25", "%f", 1, -2.25f));
    _TEST_TRUE(scans_float("+7", "%f", 1, 7.0f));
    _TEST_TRUE(scans_float("0", "%f", 1, 0.0f));
    _TEST_TRUE(scans_float("-0", "%f", 1, -0.0f));
    _TEST_TRUE(scans_float(".5", "%f", 1, 0.5f));
    _TEST_TRUE(scans_float("3.", "%f", 1, 3.0f));
    _TEST_TRUE(scans_float(" \t\n 8.5", "%f", 1, 8.5f));
    _TEST_TRUE(scans_float("1e3", "%f", 1, 1000.0f));
    _TEST_TRUE(scans_float("-.5E+2", "%f", 1, -50.0f));
    _TEST_TRUE(scans_float("25e-2", "%f", 1, 0.25f));
    _TEST_TRUE(scans_float("54.32E-1", "%f", 1, 5.432f));
    _TEST_TRUE(scans_float("1.e2", "%f", 1, 100.0f));
    _TEST_TRUE(scans_float("0x1.8p1", "%f", 1, 3.0f));
    _TEST_TRUE(scans_float("0X.8P+2", "%f", 1, 2.0f));
    _TEST_TRUE(scans_float("-0x10", "%f", 1, -16.0f));
    _TEST_TRUE(scans_float("0x1e5", "%f", 1, 485.0f));
    _TEST_TRUE(scans_float("1.5abc", "%f", 1, 1.5f));
    _TEST_TRUE(scans_float(ZEROS "5", "%f", 1, 5.0f));
    _TEST_TRUE(scans_float("." ZEROS "5e65", "%f", 1, 5.0f));
    _TEST_TRUE(scans_float("5" ZEROS "e-64", "%f", 1, 5.0f));
    _TEST_TRUE(scans_float("12345", "%3f", 1, 123.0f));
    _TEST_TRUE(scans_float("1.2345", "%2f", 1, 1.0f));
    _TEST_TRUE(scans_float("-1.25", "%4f", 1, -1.2f));
    _TEST_TRUE(scans_float("1.5", "%9f", 1, 1.5f));
    _TEST_INT(sscanf("1.5 -2.5", "%f%f", &first, &second), 2);
    _TEST_FLOAT(first, 1.5f);
    _TEST_FLOAT(second, -2.5f);
    _TEST_INT(sscanf("1.5-2.5", "%f%f", &first, &second), 2);
    _TEST_FLOAT(first, 1.5f);
    _TEST_FLOAT(second, -2.5f);
    _TEST_INT(sscanf("1.25.5", "%f%f", &first, &second), 2);
    _TEST_FLOAT(first, 1.25f);
    _TEST_FLOAT(second, 0.5f);
    _TEST_INT(sscanf("12.5e1", "%3f%f", &first, &second), 2);
    _TEST_FLOAT(first, 12.0f);
    _TEST_FLOAT(second, 50.0f);
}

int main(void)
{
    Test_StdioScanReal();
    return _Test_Status();
}
