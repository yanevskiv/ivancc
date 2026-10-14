// (Test) Status: 0
// A test file for <stdlib.h> multibyte characters in UTF-8.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define UTF8 "C.UTF-8"

// In UTF-8 a character should be one to four bytes, a stray none.
// [C99:7.20.7p1]: the multibyte functions, by LC_CTYPE
static void Test_StdlibUtf8(void)
{
    wchar_t wc = L'?';
    char bytes[TEXT];
    wchar_t wide[TEXT];

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(MB_CUR_MAX >= 1 && MB_CUR_MAX <= MB_LEN_MAX);
    _TEST_INT(mbtowc(&wc, "\xc3\xa9", 2), 2);
    _TEST_INT(wc, 0xe9);
    _TEST_INT(mblen("\xe2\x82\xac", 3), 3);
    _TEST_INT(mblen("\xc3\xa9", 1), -1);
    _TEST_INT(mbtowc(&wc, "\xff", 1), -1);
    _TEST_INT(mbtowc(&wc, "\xc3" "A", 2), -1);
    _TEST_INT(wctomb(bytes, 0x20ac), 3);
    _TEST_MEM(bytes, "\xe2\x82\xac", 3);
    _TEST_UINT(mbstowcs(wide, "a\xc3\xa9\xe2\x82\xac", 8), 3);
    _TEST_INT(wide[0], L'a');
    _TEST_INT(wide[1], 0xe9);
    _TEST_INT(wide[2], 0x20ac);
    _TEST_INT(wide[3], L'\0');
    _TEST_UINT(wcstombs(bytes, wide, sizeof(bytes)), 6);
    _TEST_STR(bytes, "a\xc3\xa9\xe2\x82\xac");
    _TEST_UINT(wcstombs(bytes, wide, 2), 1);
    _TEST_UINT(mbstowcs(wide, "a\xff", 8), (size_t) -1);
    _TEST_UINT(wcstombs(bytes, L"", 1), 0);
    _TEST_INT(bytes[0], '\0');
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_StdlibUtf8();
    return _Test_Status();
}
