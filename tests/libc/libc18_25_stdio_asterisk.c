// (Test) Status: 0
// A test file for <stdio.h> widths and precisions by argument.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.1p5] An asterisk should take a width or a precision from an int.
static void Test_StdioAsterisk(void)
{
    _TEST_TRUE(prints("  abc", "%*s", 5, "abc"));
    _TEST_TRUE(prints("abc", "%*s", 0, "abc"));
    _TEST_TRUE(prints("abc  ", "%*s", -5, "abc"));
    _TEST_TRUE(prints("abc  ", "%-*s", 5, "abc"));
    _TEST_TRUE(prints("abc  ", "%-*s", -5, "abc"));
    _TEST_TRUE(prints("  a", "%*c", 3, 'a'));
    _TEST_TRUE(prints("a  ", "%*c", -3, 'a'));

    _TEST_TRUE(prints("ab", "%.*s", 2, "abc"));
    _TEST_TRUE(prints("", "%.*s", 0, "abc"));
    _TEST_TRUE(prints("abc", "%.*s", -1, "abc"));
    _TEST_TRUE(prints("abc", "%.*s", -2, "abc"));

    _TEST_TRUE(prints("   ab", "%*.*s", 5, 2, "abc"));
    _TEST_TRUE(prints("ab   ", "%*.*s", -5, 2, "abc"));
    _TEST_TRUE(prints("  abc", "%*.*s", 5, -2, "abc"));
    _TEST_TRUE(prints("   ab", "%5.*s", 2, "abc"));
    _TEST_TRUE(prints("   ab", "%*.2s", 5, "abc"));
    _TEST_TRUE(prints(" a b", "%*s%*s", 2, "a", 2, "b"));
}

int main(void)
{
    Test_StdioAsterisk();
    return _Test_Status();
}
