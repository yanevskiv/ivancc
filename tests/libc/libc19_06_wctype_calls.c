// (Test) Status: 0
// A test file for <wctype.h> calls past a macro.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// [C99 S7.1.4p1] A call past a macro should judge and map as the function.
static void Test_WctypeCalls(void)
{
    wint_t wc = L'0';

    _TEST_TRUE((iswalnum)(L'a') && ! (iswalnum)(L'_'));
    _TEST_TRUE((iswalpha)(L'a') && ! (iswalpha)(L'7'));
    _TEST_TRUE((iswblank)(L' ') && ! (iswblank)(L'\n'));
    _TEST_TRUE((iswcntrl)(L'\n') && ! (iswcntrl)(L' '));
    _TEST_TRUE((iswdigit)(L'7') && ! (iswdigit)(L'a'));
    _TEST_TRUE((iswgraph)(L'a') && ! (iswgraph)(L' '));
    _TEST_TRUE((iswlower)(L'a') && ! (iswlower)(L'A'));
    _TEST_TRUE((iswprint)(L' ') && ! (iswprint)(L'\t'));
    _TEST_TRUE((iswpunct)(L'!') && ! (iswpunct)(L'a'));
    _TEST_TRUE((iswspace)(L'\v') && ! (iswspace)(L'a'));
    _TEST_TRUE((iswupper)(L'A') && ! (iswupper)(L'a'));
    _TEST_TRUE((iswxdigit)(L'f') && ! (iswxdigit)(L'g'));
    _TEST_TRUE((iswctype)(L'a', (wctype)("alpha")));
    _TEST_TRUE((towlower)(L'Q') == L'q');
    _TEST_TRUE((towupper)(L'q') == L'Q');
    _TEST_TRUE((towctrans)(L'q', (wctrans)("toupper")) == L'Q');

    _TEST_TRUE(iswdigit(wc++));
    _TEST_TRUE(wc == L'1');
    _TEST_TRUE(towupper(wc++) == L'1');
    _TEST_TRUE(wc == L'2');
}

int main(void)
{
    Test_WctypeCalls();
    return _Test_Status();
}
