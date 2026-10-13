// (Test) Status: 0
// A test file for <wchar.h> wide string calls past a macro.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// (S7.1.4p1) A pointer and a call in parentheses should reach each function.
static void Test_WcharCalls(void)
{
    wchar_t buf[SIZE];
    wchar_t *place;
    size_t (*length)(const wchar_t *) = wcslen;
    wchar_t *(*copy)(wchar_t *restrict, const wchar_t *restrict, size_t) = wmemcpy;

    _TEST_TRUE(copy(buf, L"zy", 3) == buf);
    _TEST_MEM(buf, L"zy", WIDE(3));
    _TEST_UINT(length(L"abcd"), 4);

    _TEST_TRUE((wmemcpy)(buf, L"ab", 3) == buf);
    _TEST_MEM(buf, L"ab", WIDE(3));
    _TEST_TRUE((wmemmove)(buf + 1, buf, 2) == buf + 1);
    _TEST_MEM(buf, L"aab", WIDE(3));
    _TEST_TRUE((wcscpy)(buf, L"abc") == buf);
    _TEST_MEM(buf, L"abc", WIDE(4));
    _TEST_TRUE((wcsncpy)(buf, L"xy", 2) == buf);
    _TEST_MEM(buf, L"xyc", WIDE(4));
    _TEST_TRUE((wcscat)(buf, L"d") == buf);
    _TEST_MEM(buf, L"xycd", WIDE(5));
    _TEST_TRUE((wcsncat)(buf, L"efg", 1) == buf);
    _TEST_MEM(buf, L"xycde", WIDE(6));
    _TEST_INT((wmemcmp)(L"a", L"a", 1), 0);
    _TEST_INT((wcscmp)(L"a", L"a"), 0);
    _TEST_INT((wcscoll)(L"a", L"a"), 0);
    _TEST_INT((wcsncmp)(L"ab", L"ac", 1), 0);
    _TEST_UINT((wcsxfrm)(NULL, L"", 0), 0);
    _TEST_TRUE((wmemchr)(buf, L'c', 5) == buf + 2);
    _TEST_TRUE((wcschr)(buf, L'c') == buf + 2);
    _TEST_UINT((wcscspn)(buf, L"c"), 2);
    _TEST_TRUE((wcspbrk)(buf, L"dc") == buf + 2);
    _TEST_TRUE((wcsrchr)(buf, L'c') == buf + 2);
    _TEST_UINT((wcsspn)(buf, L"yx"), 2);
    _TEST_TRUE((wcsstr)(buf, L"cd") == buf + 2);
    _TEST_TRUE((wcstok)(buf, L"c", &place) == buf);
    _TEST_UINT((wcslen)(buf), 2);
    _TEST_TRUE((wmemset)(buf, L'q', 1) == buf);
    _TEST_TRUE(buf[0] == L'q');
}

int main(void)
{
    Test_WcharCalls();
    return _Test_Status();
}
