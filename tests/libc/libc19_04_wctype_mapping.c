// (Test) Status: 0
// A test file for <wctype.h> case mappings.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// [C99 S7.25.3.1] A mapping should take ASCII as <ctype.h>'s: the first to differ.
static long mismapped(wint_t (*map)(wint_t), int (*basic)(int))
{
    wint_t wc;

    for (wc = 0; wc <= ASCII; wc++) {
        if (map(wc) != (wint_t) basic((int) wc)) {
            return (long) wc;
        }
    }
    return NONE;
}

// [C99 S7.25.3.1] A mapping should give a letter's other case and leave any other.
static void Test_WctypeMapping(void)
{
    _TEST_INT(mismapped(towlower, tolower), NONE);
    _TEST_INT(mismapped(towupper, toupper), NONE);
    _TEST_TRUE(towlower(L'Q') == L'q');
    _TEST_TRUE(towlower(L'q') == L'q');
    _TEST_TRUE(towlower(L'7') == L'7');
    _TEST_TRUE(towlower(L'[') == L'[');
    _TEST_TRUE(towupper(L'q') == L'Q');
    _TEST_TRUE(towupper(L'Q') == L'Q');
    _TEST_TRUE(towupper(L'7') == L'7');
    _TEST_TRUE(towupper(L'{') == L'{');
}

// [C99 S7.25.3.1] A mapping should return what is no letter unchanged, WEOF too.
static void Test_WctypeMappingWeof(void)
{
    _TEST_TRUE(towlower(WEOF) == WEOF);
    _TEST_TRUE(towupper(WEOF) == WEOF);
}

int main(void)
{
    Test_WctypeMapping();
    Test_WctypeMappingWeof();
    return _Test_Status();
}
