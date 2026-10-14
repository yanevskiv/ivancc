// (Test) Status: 0
// A test file for <stdio.h> the lc conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p8] %lc should write a wide character as %ls writes it alone.
static bool alone(wchar_t wc)
{
    int n1;
    int n2;
    int lost = 0;
    char got[SMALL];
    char want[SMALL];
    wchar_t pair[2] = {wc, L'\0'};

    n1 = snprintf(got, sizeof(got), "[%lc]", wc);
    n2 = snprintf(want, sizeof(want), "[%ls]", pair);
    lost += ! _TEST_STR(got, want);
    lost += ! _TEST_INT(n1, n2);
    return lost == 0;
}

// [C99:7.19.6.1p8] %lc should write a wide character's bytes, one if it is basic.
static void Test_StdioWideCharacter(void)
{
    _TEST_TRUE(prints("a", "%lc", L'a'));
    _TEST_TRUE(prints("abc", "%lc%lc%lc", L'a', L'b', L'c'));
    _TEST_TRUE(prints(" ", "%lc", L' '));
    _TEST_TRUE(prints("%", "%lc", L'%'));
    _TEST_TRUE(prints("ab", "%c%lc", 'a', L'b'));
    _TEST_TRUE(prints("a1b", "%lc%d%lc", L'a', 1, L'b'));
    _TEST_TRUE(prints("a\nb", "%lc%lc%lc", L'a', L'\n', L'b'));

    _TEST_TRUE(alone(L'a'));
    _TEST_TRUE(alone(L'Z'));
    _TEST_TRUE(alone(L'0'));
    _TEST_TRUE(alone(L'~'));
    _TEST_TRUE(alone(L'\t'));
}

int main(void)
{
    Test_StdioWideCharacter();
    return _Test_Status();
}
