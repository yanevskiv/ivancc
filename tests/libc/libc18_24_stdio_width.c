// (Test) Status: 0
// A test file for <stdio.h> field widths.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.1p4] A width should pad a shorter conversion with spaces on a side.
static void Test_StdioWidth(void)
{
    _TEST_TRUE(prints("  abc", "%5s", "abc"));
    _TEST_TRUE(prints("abc  ", "%-5s", "abc"));
    _TEST_TRUE(prints("abc", "%3s", "abc"));
    _TEST_TRUE(prints("abc", "%2s", "abc"));
    _TEST_TRUE(prints("     ", "%5s", ""));
    _TEST_TRUE(prints("          abc", "%13s", "abc"));

    _TEST_TRUE(prints("  a", "%3c", 'a'));
    _TEST_TRUE(prints("a  ", "%-3c", 'a'));
    _TEST_TRUE(prints("a", "%1c", 'a'));

    _TEST_TRUE(prints("   ab", "%5.2s", "abc"));
    _TEST_TRUE(prints("ab   ", "%-5.2s", "abc"));
    _TEST_TRUE(prints("     ", "%5.0s", "abc"));
    _TEST_TRUE(prints("[  a][b  ]", "[%3s][%-3s]", "a", "b"));
}

int main(void)
{
    Test_StdioWidth();
    return _Test_Status();
}
