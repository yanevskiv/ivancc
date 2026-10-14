// (Test) Status: 0
// A test file for <stdio.h> the lc conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %lc should read characters into wide ones, adding no null.
// %lc should skip no white space.
// [C99:7.19.6.2p12]: scanf's conversion specifiers
// [C99:7.19.6.2p8]:  white space skipped but for [, c and n
static void Test_StdioScanWideChar(void)
{
    char ch = FILL;
    wchar_t first = FILL;
    wchar_t second = FILL;

    _TEST_TRUE(scans_wide("abc", "%lc", 1, L"a", 1));
    _TEST_TRUE(scans_wide("abc", "%1lc", 1, L"a", 1));
    _TEST_TRUE(scans_wide("abc", "%2lc", 1, L"ab", 2));
    _TEST_TRUE(scans_wide("abc", "%3lc", 1, L"abc", 3));
    _TEST_TRUE(scans_wide(" a", "%lc", 1, L" ", 1));
    _TEST_TRUE(scans_wide("\n\tb", "%3lc", 1, L"\n\tb", 3));
    _TEST_TRUE(scans_wide(" a", " %lc", 1, L"a", 1));
    _TEST_TRUE(scans_wide("~", "%lc", 1, L"~", 1));
    _TEST_TRUE(scans_wide("", "%lc", EOF, L"", 0));
    _TEST_TRUE(scans_wide("", "%4lc", EOF, L"", 0));
    _TEST_INT(sscanf("xyz", "%lc%lc%c", &first, &second, &ch), 3);
    _TEST_INT(first, L'x');
    _TEST_INT(second, L'y');
    _TEST_INT(ch, 'z');
    _TEST_INT(sscanf("p q", "%lc %lc", &first, &second), 2);
    _TEST_INT(first, L'p');
    _TEST_INT(second, L'q');
    _TEST_INT(sscanf("r", "%lc%lc", &first, &second), 1);
    _TEST_INT(first, L'r');
    _TEST_INT(second, L'q');
}

int main(void)
{
    Test_StdioScanWideChar();
    return _Test_Status();
}
