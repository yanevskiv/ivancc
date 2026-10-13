// (Test) Status: 0
// A test file for <wchar.h> wide string copying functions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99/S7.24.4.2] A copy should write what is asked for alone, and return where.
static void Test_WcharCopying(void)
{
    wchar_t buf[SIZE];

    put(buf, L"xxxxxxxx", FILLED);
    _TEST_TRUE(wcscpy(buf, L"abc") == buf);
    _TEST_MEM(buf, L"abc\0xxxx", WIDE(FILLED));
    _TEST_TRUE(wcscpy(buf, L"") == buf);
    _TEST_MEM(buf, L"\0bc\0xxxx", WIDE(FILLED));

    put(buf, L"xxxxxxxx", FILLED);
    _TEST_TRUE(wcsncpy(buf, L"ab", 5) == buf);
    _TEST_MEM(buf, L"ab\0\0\0xxx", WIDE(FILLED));
    _TEST_TRUE(wcsncpy(buf, L"abcdefg", 3) == buf);
    _TEST_MEM(buf, L"abc\0\0xxx", WIDE(FILLED));
    _TEST_TRUE(wcsncpy(buf, L"z", 0) == buf);
    _TEST_MEM(buf, L"abc\0\0xxx", WIDE(FILLED));

    put(buf, L"xxxxxxxx", FILLED);
    _TEST_TRUE(wmemcpy(buf, L"ab\0cd", 5) == buf);
    _TEST_MEM(buf, L"ab\0cdxxx", WIDE(FILLED));
    _TEST_TRUE(wmemcpy(buf, L"z", 0) == buf);
    _TEST_MEM(buf, L"ab\0cdxxx", WIDE(FILLED));

    put(buf, L"abcdefgh", FILLED);
    _TEST_TRUE(wmemmove(buf + 2, buf, 5) == buf + 2);
    _TEST_MEM(buf, L"ababcdeh", WIDE(FILLED));
    put(buf, L"abcdefgh", FILLED);
    _TEST_TRUE(wmemmove(buf, buf + 2, 5) == buf);
    _TEST_MEM(buf, L"cdefgfgh", WIDE(FILLED));
    _TEST_TRUE(wmemmove(buf, buf, FILLED) == buf);
    _TEST_MEM(buf, L"cdefgfgh", WIDE(FILLED));
}

// [C99/S7.24.4.2] A copy should carry a wide character of any value whole.
static void Test_WcharCopyingValues(void)
{
    static const wchar_t from[] = {0x20AC, 0x10FFFF, WCHAR_MAX, WCHAR_MIN, L'\0'};
    wchar_t buf[SIZE];

    _TEST_TRUE(wcscpy(buf, from) == buf);
    _TEST_MEM(buf, from, sizeof(from));
    _TEST_TRUE(wmemcpy(buf + 8, from, 5) == buf + 8);
    _TEST_MEM(buf + 8, from, sizeof(from));
}

int main(void)
{
    Test_WcharCopying();
    Test_WcharCopyingValues();
    return _Test_Status();
}
