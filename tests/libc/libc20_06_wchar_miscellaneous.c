// (Test) Status: 0
// A test file for <wchar.h> miscellaneous wide string functions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99 S7.24.4.6] wcslen should count to the null, and wmemset fill with a value.
static void Test_WcharMiscellaneous(void)
{
    wchar_t buf[SIZE];

    _TEST_UINT(wcslen(L""), 0);
    _TEST_UINT(wcslen(L"abc"), 3);
    _TEST_UINT(wcslen(L"ab\0c"), 2);
    _TEST_UINT(wcslen(L"\x20AC\x10FFFF"), 2);

    put(buf, L"xxxxxxxx", FILLED);
    _TEST_TRUE(wmemset(buf, L'a', 3) == buf);
    _TEST_MEM(buf, L"aaaxxxxx", WIDE(FILLED));
    _TEST_TRUE(wmemset(buf, 0x20AC, 2) == buf);
    _TEST_MEM(buf, L"\x20AC\x20AC" L"axxxxx", WIDE(FILLED));
    _TEST_TRUE(wmemset(buf, L'c', 0) == buf);
    _TEST_MEM(buf, L"\x20AC\x20AC" L"axxxxx", WIDE(FILLED));
}

int main(void)
{
    Test_WcharMiscellaneous();
    return _Test_Status();
}
