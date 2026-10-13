// (Test) Status: 0
// A test file for <wctype.h> classes of the basic characters.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

// [C99 S7.25.2.1p1] A class should judge ASCII as <ctype.h>'s: the first to differ.
static long misjudged(int (*classify)(wint_t), int (*basic)(int))
{
    wint_t wc;

    for (wc = 0; wc <= ASCII; wc++) {
        if ((classify(wc) != 0) != (basic((int) wc) != 0)) {
            return (long) wc;
        }
    }
    return NONE;
}

// [C99 S7.25.2.1p1] Each class should hold of the characters its narrow one does.
static void Test_WctypeClasses(void)
{
    int i;

    for (i = 0; i < CLASSES; i++) {
        _TEST_INT(misjudged(wide[i], narrow[i]), NONE);
    }
}

// [C99 S7.25.2.1] A class should be true of its own characters and false of others.
static void Test_WctypeClassesNamed(void)
{
    _TEST_TRUE(iswalnum(L'a') && iswalnum(L'7') && ! iswalnum(L'_'));
    _TEST_TRUE(iswalpha(L'a') && iswalpha(L'Z') && ! iswalpha(L'7'));
    _TEST_TRUE(iswblank(L' ') && iswblank(L'\t') && ! iswblank(L'\n'));
    _TEST_TRUE(iswcntrl(L'\n') && iswcntrl(L'\177') && ! iswcntrl(L' '));
    _TEST_TRUE(iswdigit(L'0') && iswdigit(L'9') && ! iswdigit(L'a'));
    _TEST_TRUE(iswgraph(L'a') && iswgraph(L'~') && ! iswgraph(L' '));
    _TEST_TRUE(iswlower(L'a') && iswlower(L'z') && ! iswlower(L'A'));
    _TEST_TRUE(iswprint(L'a') && iswprint(L' ') && ! iswprint(L'\t'));
    _TEST_TRUE(iswpunct(L'!') && iswpunct(L'~') && ! iswpunct(L'a'));
    _TEST_TRUE(iswspace(L' ') && iswspace(L'\v') && ! iswspace(L'a'));
    _TEST_TRUE(iswupper(L'A') && iswupper(L'Z') && ! iswupper(L'a'));
    _TEST_TRUE(iswxdigit(L'9') && iswxdigit(L'f') && iswxdigit(L'F') && ! iswxdigit(L'g'));
}

// [C99 S7.25.2.1] The null wide character should be of the control class alone.
static void Test_WctypeNull(void)
{
    int i;

    for (i = 0; i < CLASSES; i++) {
        _TEST_TRUE((wide[i](L'\0') != 0) == (wide[i] == iswcntrl));
    }
}

int main(void)
{
    Test_WctypeClasses();
    Test_WctypeClassesNamed();
    Test_WctypeNull();
    return _Test_Status();
}
