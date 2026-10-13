// (Test) Status: 0
// A test file for <stdio.h> precisions of integers.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.1p8) A precision should be the fewest digits, zeros making them up.
static void Test_StdioDigits(void)
{
    _TEST_TRUE(prints("005", "%.3d", 5));
    _TEST_TRUE(prints("-005", "%.3d", -5));
    _TEST_TRUE(prints("12345", "%.3d", 12345));
    _TEST_TRUE(prints("-12345", "%.3d", -12345));
    _TEST_TRUE(prints("5", "%.1d", 5));
    _TEST_TRUE(prints("0", "%.1d", 0));
    _TEST_TRUE(prints("000", "%.3d", 0));
    _TEST_TRUE(prints("007", "%.3i", 7));
    _TEST_TRUE(prints("00000000000000000042", "%.20d", 42));

    _TEST_TRUE(prints("005", "%.3u", 5U));
    _TEST_TRUE(prints("017", "%.3o", 15U));
    _TEST_TRUE(prints("00ff", "%.4x", 255U));
    _TEST_TRUE(prints("00FF", "%.4X", 255U));
    _TEST_TRUE(prints("ff", "%.1x", 255U));

    _TEST_TRUE(prints("", "%.0d", 0));
    _TEST_TRUE(prints("", "%.d", 0));
    _TEST_TRUE(prints("", "%.0i", 0));
    _TEST_TRUE(prints("", "%.0u", 0U));
    _TEST_TRUE(prints("", "%.0o", 0U));
    _TEST_TRUE(prints("", "%.0x", 0U));
    _TEST_TRUE(prints("", "%.0X", 0U));
    _TEST_TRUE(prints("5", "%.0d", 5));
    _TEST_TRUE(prints("5", "%.d", 5));
    _TEST_TRUE(prints("[]", "[%.0d]", 0));

    _TEST_TRUE(prints("  005", "%5.3d", 5));
    _TEST_TRUE(prints(" -005", "%5.3d", -5));
    _TEST_TRUE(prints("005  ", "%-5.3d", 5));
    _TEST_TRUE(prints("-005 ", "%-5.3d", -5));
    _TEST_TRUE(prints("     ", "%5.0d", 0));
    _TEST_TRUE(prints(" 00ff", "%5.4x", 255U));
    _TEST_TRUE(prints("00005", "%3.5d", 5));

    _TEST_TRUE(prints("005", "%.*d", 3, 5));
    _TEST_TRUE(prints("", "%.*d", 0, 0));
    _TEST_TRUE(prints("5", "%.*d", -3, 5));
    _TEST_TRUE(prints("0", "%.*d", -1, 0));
    _TEST_TRUE(prints("  005", "%*.*d", 5, 3, 5));
    _TEST_TRUE(prints("005  ", "%*.*d", -5, 3, 5));
}

int main(void)
{
    Test_StdioDigits();
    return _Test_Status();
}
