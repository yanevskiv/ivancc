// (Test) Status: 0
// A test file for <wctype.h> classes by name.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// A class by its name should judge as its function does.
// [C99:7.25.2.2.1p4]: iswctype's true, nonzero
static long misjudged(wctype_t class, int (*classify)(wint_t))
{
    wint_t wc;

    for (wc = 0; wc <= ASCII; wc++) {
        if ((iswctype(wc, class) != 0) != (classify(wc) != 0)) {
            return (long) wc;
        }
    }
    return NONE;
}

// Each of the twelve names should describe its class.
// [C99:7.25.2.2.2p3]: wctype's twelve class names, valid in every locale
static void Test_WctypeNames(void)
{
    wctype_t class;
    int i;

    for (i = 0; i < CLASSES; i++) {
        class = wctype(names[i]);
        _TEST_TRUE(class != 0);
        _TEST_INT(misjudged(class, wide[i]), NONE);
    }
}

// A name of no class should give zero.
// [C99:7.25.2.2.2p4]: wctype's zero for a class not valid in the locale
static void Test_WctypeNoName(void)
{
    _TEST_TRUE(wctype("nonsense") == 0);
    _TEST_TRUE(wctype("alphabet") == 0);
    _TEST_TRUE(wctype("alph") == 0);
    _TEST_TRUE(wctype("") == 0);
}

// A class by its name should hold of its characters alone.
// [C99:7.25.2.2.1]: iswctype
static void Test_WctypeByName(void)
{
    _TEST_TRUE(iswctype(L'a', wctype("alpha")));
    _TEST_TRUE(! iswctype(L'7', wctype("alpha")));
    _TEST_TRUE(iswctype(L'7', wctype("digit")));
    _TEST_TRUE(! iswctype(L'a', wctype("digit")));
    _TEST_TRUE(iswctype(L'f', wctype("xdigit")));
    _TEST_TRUE(! iswctype(L'g', wctype("xdigit")));
    _TEST_TRUE(iswctype(L' ', wctype("space")));
    _TEST_TRUE(! iswctype(L' ', wctype("graph")));
    _TEST_TRUE(iswctype(L' ', wctype("print")));
}

int main(void)
{
    Test_WctypeNames();
    Test_WctypeNoName();
    Test_WctypeByName();
    return _Test_Status();
}
