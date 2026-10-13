// (Test) Status: 0
// A test file for <stdio.h> the # flag on integers.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.1p6] A # flag should lead octal with a zero and hexadecimal with 0x.
static void Test_StdioAlternate(void)
{
    _TEST_TRUE(prints("010", "%#o", 8U));
    _TEST_TRUE(prints("0777", "%#o", 511U));
    _TEST_TRUE(prints("0", "%#o", 0U));
    _TEST_TRUE(prints("0", "%#.0o", 0U));
    _TEST_TRUE(prints("01", "%#.0o", 1U));
    _TEST_TRUE(prints("01", "%#.1o", 1U));
    _TEST_TRUE(prints("01", "%#.2o", 1U));
    _TEST_TRUE(prints("001", "%#.3o", 1U));
    _TEST_TRUE(prints("0010", "%#.4o", 8U));
    _TEST_TRUE(prints("010", "%#.2o", 8U));
    _TEST_TRUE(prints("037777777777", "%#o", UINT_MAX));

    _TEST_TRUE(prints("0xff", "%#x", 255U));
    _TEST_TRUE(prints("0XFF", "%#X", 255U));
    _TEST_TRUE(prints("0x1", "%#x", 1U));
    _TEST_TRUE(prints("0", "%#x", 0U));
    _TEST_TRUE(prints("0", "%#X", 0U));
    _TEST_TRUE(prints("", "%#.0x", 0U));
    _TEST_TRUE(prints("0x00ff", "%#.4x", 255U));
    _TEST_TRUE(prints("0X00FF", "%#.4X", 255U));
    _TEST_TRUE(prints("0xffffffff", "%#x", UINT_MAX));

    _TEST_TRUE(prints("  0xff", "%#6x", 255U));
    _TEST_TRUE(prints("0xff  ", "%-#6x", 255U));
    _TEST_TRUE(prints("  0XFF", "%#6X", 255U));
    _TEST_TRUE(prints("   010", "%#6o", 8U));
    _TEST_TRUE(prints("010   ", "%-#6o", 8U));
    _TEST_TRUE(prints("     0", "%#6x", 0U));
    _TEST_TRUE(prints(" 0x00ff", "%#7.4x", 255U));
    _TEST_TRUE(prints("0xff", "%#2x", 255U));
}

int main(void)
{
    Test_StdioAlternate();
    return _Test_Status();
}
