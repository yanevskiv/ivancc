// (Test) Status: 0
// A test file for <stdio.h> the plus and space flags.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p6] A plus or a space flag should begin a signed conversion.
static void Test_StdioSign(void)
{
    _TEST_TRUE(prints("+5", "%+d", 5));
    _TEST_TRUE(prints("-5", "%+d", -5));
    _TEST_TRUE(prints("+0", "%+d", 0));
    _TEST_TRUE(prints("+5", "%+i", 5));
    _TEST_TRUE(prints("+2147483647", "%+d", INT_MAX));
    _TEST_TRUE(prints("-2147483648", "%+d", INT_MIN));

    _TEST_TRUE(prints(" 5", "% d", 5));
    _TEST_TRUE(prints("-5", "% d", -5));
    _TEST_TRUE(prints(" 0", "% d", 0));
    _TEST_TRUE(prints(" 5", "% i", 5));
    _TEST_TRUE(prints(" ", "% .0d", 0));
    _TEST_TRUE(prints("+", "%+.0d", 0));

    _TEST_TRUE(prints("+5", "%+ d", 5));
    _TEST_TRUE(prints("+5", "% +d", 5));
    _TEST_TRUE(prints("-5", "%+ d", -5));

    _TEST_TRUE(prints("   +5", "%+5d", 5));
    _TEST_TRUE(prints("+5   ", "%-+5d", 5));
    _TEST_TRUE(prints("+5   ", "%+-5d", 5));
    _TEST_TRUE(prints("    5", "% 5d", 5));
    _TEST_TRUE(prints(" 5   ", "%- 5d", 5));
    _TEST_TRUE(prints("+005", "%+.3d", 5));
    _TEST_TRUE(prints(" 005", "% .3d", 5));
    _TEST_TRUE(prints(" +005", "%+5.3d", 5));
    _TEST_TRUE(prints("+1 -2  3", "%+d %+d % d", 1, -2, 3));
}

int main(void)
{
    Test_StdioSign();
    return _Test_Status();
}
