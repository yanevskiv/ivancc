// (Test) Status: 0
// A test file for <wctype.h> relations between the classes in UTF-8.

// Include helpers.
#include "libc19_00_wctype_helpers.inc"

#define UTF8  "C.UTF-8"
#define DENSE 0x2FF
#define LAST  0x10FFFF
#define STEP  4093

// Return the code after wc of those checked: each to DENSE, then one in STEP.
static wint_t next(wint_t wc)
{
    return wc < DENSE ? wc + 1 : wc + STEP;
}

// Tell whether wc is one of the hexadecimal digits [C99:S7.25.2.1.12] lists.
static bool hex(wint_t wc)
{
    return (wc >= L'0' && wc <= L'9') || (wc >= L'a' && wc <= L'f') || (wc >= L'A' && wc <= L'F');
}

// [C99:S7.25.2.1] Return the first code whose classes break what each must hold.
static long broken(void)
{
    wint_t wc;

    for (wc = 0; wc <= LAST; wc = next(wc)) {
        bool other = iswcntrl(wc) || iswdigit(wc) || iswpunct(wc) || iswspace(wc);

        if ((iswalnum(wc) != 0) != (iswalpha(wc) || iswdigit(wc))) {
            return (long) wc;
        }
        if ((iswupper(wc) || iswlower(wc)) && ! iswalpha(wc)) {
            return (long) wc;
        }
        if ((iswalpha(wc) || iswlower(wc) || iswupper(wc)) && other) {
            return (long) wc;
        }
        if ((iswdigit(wc) != 0) != (wc >= L'0' && wc <= L'9')) {
            return (long) wc;
        }
        if ((iswgraph(wc) != 0) != (iswprint(wc) && ! iswspace(wc))) {
            return (long) wc;
        }
        if (iswpunct(wc) && (! iswprint(wc) || iswspace(wc) || iswalnum(wc))) {
            return (long) wc;
        }
        if (iswspace(wc) && (iswalnum(wc) || iswgraph(wc) || iswpunct(wc))) {
            return (long) wc;
        }
        if ((iswxdigit(wc) != 0) != hex(wc)) {
            return (long) wc;
        }
        if (iswblank(wc) && ! iswspace(wc)) {
            return (long) wc;
        }
    }
    return NONE;
}

// [C99:S7.25.3.1] Return the first code a mapping moves against its classes.
static long mismapped(void)
{
    wctrans_t lower = wctrans("tolower");
    wctrans_t upper = wctrans("toupper");
    wint_t wc;

    for (wc = 0; wc <= LAST; wc = next(wc)) {
        if (towlower(wc) != wc && ! (iswupper(wc) && iswlower(towlower(wc)))) {
            return (long) wc;
        }
        if (towupper(wc) != wc && ! (iswlower(wc) && iswupper(towupper(wc)))) {
            return (long) wc;
        }
        if (towctrans(wc, lower) != towlower(wc)) {
            return (long) wc;
        }
        if (towctrans(wc, upper) != towupper(wc)) {
            return (long) wc;
        }
    }
    return NONE;
}

// [C99:S7.25.2.2.1p4] Return the first code a class by name judges otherwise.
static long misnamed(void)
{
    wctype_t class;
    wint_t wc;
    int i;

    for (i = 0; i < CLASSES; i++) {
        class = wctype(names[i]);
        for (wc = 0; wc <= LAST; wc = next(wc)) {
            if ((iswctype(wc, class) != 0) != (wide[i](wc) != 0)) {
                return (long) wc;
            }
        }
    }
    return NONE;
}

// [C99:S7.25.2.1] In UTF-8 the classes and mappings should hold as in any locale.
static void Test_WctypeRelations(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_INT(broken(), NONE);
    _TEST_INT(mismapped(), NONE);
    _TEST_INT(misnamed(), NONE);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
    _TEST_INT(broken(), NONE);
    _TEST_INT(mismapped(), NONE);
    _TEST_INT(misnamed(), NONE);
}

int main(void)
{
    Test_WctypeRelations();
    return _Test_Status();
}
