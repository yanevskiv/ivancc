// (Test) Status: 0
// A test file for <stdio.h> %f and %F.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %f and %F should write [-]ddd.ddd, six digits after the point.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static void Test_StdioFixed(void)
{
    _TEST_TRUE(prints("0.000000", "%f", 0.0));
    _TEST_TRUE(prints("-0.000000", "%f", -0.0));
    _TEST_TRUE(prints("1.000000", "%f", 1.0));
    _TEST_TRUE(prints("-1.000000", "%f", -1.0));
    _TEST_TRUE(prints("1.500000", "%f", 1.5));
    _TEST_TRUE(prints("0.500000", "%f", 0.5f));
    _TEST_TRUE(prints("3.141593", "%f", 3.141592653589793));
    _TEST_TRUE(prints("-3.141593", "%f", -3.141592653589793));
    _TEST_TRUE(prints("123456789.000000", "%f", 123456789.0));
    _TEST_TRUE(prints("0.000001", "%f", 0.000001));
    _TEST_TRUE(prints("0.000000", "%f", 0.0000001));
    _TEST_TRUE(prints("1.500000", "%F", 1.5));
    _TEST_TRUE(prints("-0.000000", "%F", -0.0));

    _TEST_TRUE(prints("3", "%.0f", 3.0));
    _TEST_TRUE(prints("3", "%.f", 3.0));
    _TEST_TRUE(prints("3.1", "%.1f", 3.14));
    _TEST_TRUE(prints("3.14", "%.2f", 3.14159));
    _TEST_TRUE(prints("1234.568", "%.3f", 1234.5678));
    _TEST_TRUE(prints("0.5000000000", "%.10f", 0.5));
    _TEST_TRUE(prints("0.50000000000000000000", "%.20f", 0.5));
    _TEST_TRUE(prints("0.00", "%.2f", 0.001));
    _TEST_TRUE(prints("0", "%.0f", 0.0));
    _TEST_TRUE(prints("-0", "%.0f", -0.0));

    _TEST_TRUE(prints("1000000000000000", "%.0f", 1e15));
    _TEST_TRUE(prints("10000000000000000000000", "%.0f", 1e22));
    _TEST_TRUE(prints("4294967296.0", "%.1f", 4294967296.0));
    _TEST_TRUE(prints("9007199254740992.00", "%.2f", 9007199254740992.0));

    _TEST_TRUE(prints("3.14", "%.*f", 2, 3.14159));
    _TEST_TRUE(prints("3", "%.*f", 0, 3.14159));
    _TEST_TRUE(prints("3.141590", "%.*f", -1, 3.14159));
    _TEST_TRUE(prints("[1.500000]", "[%f]", 1.5));
    _TEST_TRUE(prints("1.500000 2.500000", "%f %f", 1.5, 2.5));
}

int main(void)
{
    Test_StdioFixed();
    return _Test_Status();
}
