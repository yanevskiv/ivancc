// (Test) Status: 0
// A test file for <stdio.h> widths of the lc and ls conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.1p4) A width should pad %lc and %ls with spaces, left or right.
static void Test_StdioWideWidth(void)
{
    _TEST_TRUE(prints("  a", "%3lc", L'a'));
    _TEST_TRUE(prints("a  ", "%-3lc", L'a'));
    _TEST_TRUE(prints("a", "%1lc", L'a'));
    _TEST_TRUE(prints("  a", "%*lc", 3, L'a'));
    _TEST_TRUE(prints("a  ", "%*lc", -3, L'a'));
    _TEST_TRUE(prints("[  a][b  ]", "[%3lc][%-3lc]", L'a', L'b'));

    _TEST_TRUE(prints("   abc", "%6ls", L"abc"));
    _TEST_TRUE(prints("abc   ", "%-6ls", L"abc"));
    _TEST_TRUE(prints("abc", "%3ls", L"abc"));
    _TEST_TRUE(prints("abc", "%2ls", L"abc"));
    _TEST_TRUE(prints("   ", "%3ls", L""));
    _TEST_TRUE(prints("  abc", "%*ls", 5, L"abc"));
    _TEST_TRUE(prints("abc  ", "%*ls", -5, L"abc"));

    _TEST_TRUE(prints("    ab", "%6.2ls", L"abc"));
    _TEST_TRUE(prints("ab    ", "%-6.2ls", L"abc"));
    _TEST_TRUE(prints("      ", "%6.0ls", L"abc"));
    _TEST_TRUE(prints("   ab", "%*.*ls", 5, 2, L"abc"));
    _TEST_TRUE(prints("  abc", "%*.*ls", 5, -2, L"abc"));
}

int main(void)
{
    Test_StdioWideWidth();
    return _Test_Status();
}
