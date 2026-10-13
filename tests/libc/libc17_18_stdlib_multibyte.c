// (Test) Status: 0
// A test file for <stdlib.h> multibyte characters in the "C" locale.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.20.7p1] In the "C" locale a multibyte character should be one byte.
static void Test_StdlibMultibyte(void)
{
    wchar_t wc = L'?';
    char bytes[TEXT];
    wchar_t wide[TEXT];

    _TEST_INT(mbtowc(&wc, "A", 1), 1);
    _TEST_INT(wc, L'A');
    _TEST_INT(mbtowc(&wc, "", 1), 0);
    _TEST_INT(wc, L'\0');
    _TEST_INT(mbtowc(NULL, "A", 1), 1);
    _TEST_INT(mblen("A", 1), 1);
    _TEST_INT(mblen("", 1), 0);
    _TEST_INT(mblen("A", 0), -1);
    _TEST_INT((mblen)("AB", 2), 1);
    _TEST_INT((mbtowc)(&wc, "B", 1), 1);
    _TEST_INT(wc, L'B');
    _TEST_INT(mbtowc(NULL, NULL, 0) != 0, wctomb(NULL, L'\0') != 0);
    _TEST_INT(mblen(NULL, 0) != 0, wctomb(NULL, L'\0') != 0);
    _TEST_INT(wctomb(bytes, L'A'), 1);
    _TEST_INT(bytes[0], 'A');
    _TEST_INT((wctomb)(bytes, L'\0'), 1);
    _TEST_INT(bytes[0], '\0');

    _TEST_UINT(mbstowcs(wide, "hello", 8), 5);
    _TEST_MEM(wide, L"hello", 6 * sizeof(wchar_t));
    wide[3] = L'x';
    _TEST_UINT((mbstowcs)(wide, "abcde", 3), 3);
    _TEST_MEM(wide, L"abcx", 4 * sizeof(wchar_t));
    _TEST_UINT(wcstombs(bytes, L"hi", sizeof(bytes)), 2);
    _TEST_STR(bytes, "hi");
    bytes[3] = 'x';
    _TEST_UINT((wcstombs)(bytes, L"hello", 3), 3);
    _TEST_MEM(bytes, "helx", 4);
}

int main(void)
{
    Test_StdlibMultibyte();
    return _Test_Status();
}
