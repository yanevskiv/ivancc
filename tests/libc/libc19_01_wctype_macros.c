// (Test) Status: 0
// A test file for <wctype.h> types and WEOF.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// [C99:7.25.1p3] WEOF should be an integer constant expression.
enum { CONSTANT = WEOF != 0 };

// [C99:7.25.1p2] wint_t should hold WEOF and any wide character, told apart.
static void Test_WctypeWint(void)
{
    wint_t wc = WEOF;

    _TEST_TRUE(CONSTANT);
    _TEST_TRUE(wc == WEOF);
    _TEST_UINT(sizeof(WEOF), sizeof(wint_t));
    wc = L'a';
    _TEST_TRUE(wc == L'a');
    _TEST_TRUE(wc != WEOF);
    wc = L'\0';
    _TEST_TRUE(wc != WEOF);
    wc = (wint_t) WCHAR_MAX;
    _TEST_TRUE(wc != WEOF);
    _TEST_TRUE((wchar_t) wc == WCHAR_MAX);
    wc = (wint_t) WCHAR_MIN;
    _TEST_TRUE(wc != WEOF);
    _TEST_TRUE((wchar_t) wc == WCHAR_MIN);
}

// [C99:7.25.1p2] wctrans_t and wctype_t should be scalar types.
static void Test_WctypeScalars(void)
{
    wctrans_t mapping = 0;
    wctype_t class = 0;

    _TEST_TRUE(! mapping);
    _TEST_TRUE(! class);
    mapping = wctrans("tolower");
    class = wctype("alpha");
    _TEST_TRUE(mapping && class);
    _TEST_TRUE(mapping == wctrans("tolower"));
    _TEST_TRUE(class == wctype("alpha"));
}

int main(void)
{
    Test_WctypeWint();
    Test_WctypeScalars();
    return _Test_Status();
}
