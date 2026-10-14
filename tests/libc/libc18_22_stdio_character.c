// (Test) Status: 0
// A test file for <stdio.h> the c conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p8] %c should write its argument as an unsigned char.
static void Test_StdioCharacter(void)
{
    char got[SMALL];

    _TEST_TRUE(prints("a", "%c", 'a'));
    _TEST_TRUE(prints("abc", "%c%c%c", 'a', 'b', 'c'));
    _TEST_TRUE(prints("a", "%c", 'a' + UCHAR_MAX + 1));
    _TEST_TRUE(prints(" ", "%c", ' '));
    _TEST_TRUE(prints("%", "%c", '%'));

    _TEST_INT(sprintf(got, "a%cb", '\0'), 3);
    _TEST_MEM(got, "a\0b", 4);
    _TEST_INT(sprintf(got, "%c", UCHAR_MAX), 1);
    _TEST_INT((unsigned char) got[0], UCHAR_MAX);
}

int main(void)
{
    Test_StdioCharacter();
    return _Test_Status();
}
