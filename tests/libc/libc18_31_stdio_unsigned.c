// (Test) Status: 0
// A test file for <stdio.h> %u, %o, %x and %X.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.1p8] %u, %o, %x and %X should write an unsigned int in their bases.
static void Test_StdioUnsigned(void)
{
    _TEST_TRUE(prints("0", "%u", 0U));
    _TEST_TRUE(prints("5", "%u", 5U));
    _TEST_TRUE(prints("65535", "%u", 65535U));
    _TEST_TRUE(prints("4294967295", "%u", UINT_MAX));
    _TEST_TRUE(prints("4294967295", "%u", (unsigned) -1));

    _TEST_TRUE(prints("0", "%o", 0U));
    _TEST_TRUE(prints("7", "%o", 7U));
    _TEST_TRUE(prints("10", "%o", 8U));
    _TEST_TRUE(prints("777", "%o", 511U));
    _TEST_TRUE(prints("177777", "%o", 65535U));
    _TEST_TRUE(prints("37777777777", "%o", UINT_MAX));

    _TEST_TRUE(prints("0", "%x", 0U));
    _TEST_TRUE(prints("9", "%x", 9U));
    _TEST_TRUE(prints("a", "%x", 10U));
    _TEST_TRUE(prints("f", "%x", 15U));
    _TEST_TRUE(prints("10", "%x", 16U));
    _TEST_TRUE(prints("abcdef", "%x", 0xABCDEFU));
    _TEST_TRUE(prints("ffffffff", "%x", UINT_MAX));

    _TEST_TRUE(prints("0", "%X", 0U));
    _TEST_TRUE(prints("A", "%X", 10U));
    _TEST_TRUE(prints("ABCDEF", "%X", 0xabcdefU));
    _TEST_TRUE(prints("1A2B3C4D", "%X", 0x1a2b3c4dU));
    _TEST_TRUE(prints("FFFFFFFF", "%X", UINT_MAX));

    _TEST_TRUE(prints("   ff", "%5x", 255U));
    _TEST_TRUE(prints("ff   ", "%-5x", 255U));
    _TEST_TRUE(prints("  377", "%5o", 255U));
    _TEST_TRUE(prints("  255", "%5u", 255U));
    _TEST_TRUE(prints("12345", "%2u", 12345U));
    _TEST_TRUE(prints("255 377 ff FF", "%u %o %x %X", 255U, 255U, 255U, 255U));
}

int main(void)
{
    Test_StdioUnsigned();
    return _Test_Status();
}
