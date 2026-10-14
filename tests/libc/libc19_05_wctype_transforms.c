// (Test) Status: 0
// A test file for <wctype.h> case mappings by name.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// A mapping by its name should map as its function does.
// [C99:7.25.3.2.1p4]: towctrans's mapped value
static long mismapped(wctrans_t mapping, wint_t (*map)(wint_t))
{
    wint_t wc;

    for (wc = 0; wc <= ASCII; wc++) {
        if (towctrans(wc, mapping) != map(wc)) {
            return (long) wc;
        }
    }
    return NONE;
}

// Each of the two names should describe its mapping.
// [C99:7.25.3.2.2p3]: wctrans's names tolower and toupper, valid in every locale
static void Test_WctypeTransforms(void)
{
    wctrans_t lower = wctrans("tolower");
    wctrans_t upper = wctrans("toupper");

    _TEST_TRUE(lower != 0);
    _TEST_TRUE(upper != 0);
    _TEST_INT(mismapped(lower, towlower), NONE);
    _TEST_INT(mismapped(upper, towupper), NONE);
    _TEST_TRUE(towctrans(L'Q', lower) == L'q');
    _TEST_TRUE(towctrans(L'q', lower) == L'q');
    _TEST_TRUE(towctrans(L'q', upper) == L'Q');
    _TEST_TRUE(towctrans(L'7', upper) == L'7');
    _TEST_TRUE(towctrans(WEOF, lower) == WEOF);
    _TEST_TRUE(towctrans(WEOF, upper) == WEOF);
}

// A name of no mapping should give zero.
// [C99:7.25.3.2.2p4]: wctrans's zero for a mapping not valid in the locale
static void Test_WctypeNoTransform(void)
{
    _TEST_TRUE(wctrans("nonsense") == 0);
    _TEST_TRUE(wctrans("tolowercase") == 0);
    _TEST_TRUE(wctrans("tolow") == 0);
    _TEST_TRUE(wctrans("") == 0);
}

int main(void)
{
    Test_WctypeTransforms();
    Test_WctypeNoTransform();
    return _Test_Status();
}
