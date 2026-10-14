// (Test) Status: 0
// A test file for <stdio.h> %d and %i.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p8] %d and %i should write an int in decimal, a minus if negative.
static void Test_StdioDecimal(void)
{
    _TEST_TRUE(prints("0", "%d", 0));
    _TEST_TRUE(prints("5", "%d", 5));
    _TEST_TRUE(prints("-5", "%d", -5));
    _TEST_TRUE(prints("10", "%d", 10));
    _TEST_TRUE(prints("12345", "%d", 12345));
    _TEST_TRUE(prints("-12345", "%d", -12345));
    _TEST_TRUE(prints("32767", "%d", 32767));
    _TEST_TRUE(prints("-32767", "%d", -32767));
    _TEST_TRUE(prints("2147483647", "%d", INT_MAX));
    _TEST_TRUE(prints("-2147483648", "%d", INT_MIN));

    _TEST_TRUE(prints("0", "%i", 0));
    _TEST_TRUE(prints("42", "%i", 42));
    _TEST_TRUE(prints("-42", "%i", -42));
    _TEST_TRUE(prints("-2147483648", "%i", INT_MIN));

    _TEST_TRUE(prints("   42", "%5d", 42));
    _TEST_TRUE(prints("  -42", "%5d", -42));
    _TEST_TRUE(prints("42   ", "%-5d", 42));
    _TEST_TRUE(prints("-42  ", "%-5d", -42));
    _TEST_TRUE(prints("12345", "%3d", 12345));
    _TEST_TRUE(prints("-12345", "%3d", -12345));
    _TEST_TRUE(prints("  7", "%*d", 3, 7));
    _TEST_TRUE(prints("7  ", "%*d", -3, 7));
    _TEST_TRUE(prints("1, -2 and 3", "%d, %d and %i", 1, -2, 3));
    _TEST_TRUE(prints("a1b22c", "a%db%dc", 1, 22));
}

int main(void)
{
    Test_StdioDecimal();
    return _Test_Status();
}
