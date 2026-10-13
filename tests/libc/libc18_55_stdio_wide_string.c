// (Test) Status: 0
// A test file for <stdio.h> the ls conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.1p8) %ls should write a wide string to its null or its precision.
static void Test_StdioWideString(void)
{
    const wchar_t open[3] = {L'x', L'y', L'z'};

    _TEST_TRUE(prints("abc", "%ls", L"abc"));
    _TEST_TRUE(prints("", "%ls", L""));
    _TEST_TRUE(prints("[abc][def]", "[%ls][%ls]", L"abc", L"def"));
    _TEST_TRUE(prints("%d", "%ls", L"%d"));
    _TEST_TRUE(prints("abcdef", "%s%ls", "abc", L"def"));
    _TEST_TRUE(prints("a 1 b", "%ls %d %ls", L"a", 1, L"b"));

    _TEST_TRUE(prints("ab", "%.2ls", L"abc"));
    _TEST_TRUE(prints("abc", "%.3ls", L"abc"));
    _TEST_TRUE(prints("abc", "%.20ls", L"abc"));
    _TEST_TRUE(prints("", "%.0ls", L"abc"));
    _TEST_TRUE(prints("", "%.ls", L"abc"));
    _TEST_TRUE(prints("a", "%.01ls", L"abc"));
    _TEST_TRUE(prints("ab", "%.*ls", 2, L"abc"));
    _TEST_TRUE(prints("abc", "%.*ls", -1, L"abc"));

    _TEST_TRUE(prints("xyz", "%.3ls", open));
    _TEST_TRUE(prints("x", "%.1ls", open));
    _TEST_TRUE(prints("", "%.0ls", open));
}

int main(void)
{
    Test_StdioWideString();
    return _Test_Status();
}
