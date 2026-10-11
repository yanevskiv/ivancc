// (Test) Status: 0
// <locale.h> defines struct lconv, NULL and the categories LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC and LC_TIME, distinct constants fit for #if (S7.11p1-p3),
// setlocale, which names a category's locale for a null one, selects "C" for any, refuses a locale it lacks and changes nothing then, and takes back a name it gave, of categories that differ too (S7.11.1.1),
// and localeconv, which gives the "C" locale's formatting, a "." for the decimal point, each other string empty and each char CHAR_MAX (S7.11p2), a copy of it the program's to change,
// the program starting in the "C" locale (S7.11.1.1p4), each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <locale.h>

// (S7.11p3) Each category should differ from the next in #if.
#define DIRECTIVE 0
#if (LC_ALL != LC_COLLATE) \
    && (LC_COLLATE != LC_CTYPE) \
    && (LC_CTYPE != LC_MONETARY) \
    && (LC_MONETARY != LC_NUMERIC) \
    && (LC_NUMERIC != LC_TIME)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

#define NONE       (-1)
#define NAME_SIZE  256
#define CATEGORIES 6
#define LACKING    "no_SUCH.locale"
#define UTF8       "C.UTF-8"

// The categories, LC_ALL first.
static const int cats[CATEGORIES] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME};

// The name of the locale the program started in.
static char start[NAME_SIZE];

// (S7.11p3) The categories should be distinct: return the first to repeat.
static int repeated(void)
{
    int i;
    int j;

    for (i = 0; i < CATEGORIES; i++) {
        for (j = 0; j < i; j++) {
            if (cats[i] == cats[j]) {
                return i;
            }
        }
    }
    return NONE;
}

// (S7.11.1.1p6) Each category should get a name: return the first without.
static int refused(const char *locale)
{
    int i;

    for (i = 0; i < CATEGORIES; i++) {
        if (setlocale(cats[i], locale) == NULL) {
            return i;
        }
    }
    return NONE;
}

// Copy a category's name, which setlocale's next call may overwrite.
static bool keep(char *buf, int category)
{
    const char *name = setlocale(category, NULL);

    buf[0] = '\0';
    if (! _TEST_TRUE(name != NULL) || ! _TEST_TRUE(strlen(name) < NAME_SIZE)) {
        return false;
    }
    strcpy(buf, name);
    return true;
}

// (S7.11p2) localeconv should give the formatting of the "C" locale.
static bool plain(void)
{
    int lost = 0;
    const struct lconv *conv = localeconv();

    if (! _TEST_TRUE(conv != NULL)) {
        return false;
    }
    lost += ! _TEST_STR(conv->decimal_point, ".");
    lost += ! _TEST_STR(conv->thousands_sep, "");
    lost += ! _TEST_STR(conv->grouping, "");
    lost += ! _TEST_STR(conv->mon_decimal_point, "");
    lost += ! _TEST_STR(conv->mon_thousands_sep, "");
    lost += ! _TEST_STR(conv->mon_grouping, "");
    lost += ! _TEST_STR(conv->positive_sign, "");
    lost += ! _TEST_STR(conv->negative_sign, "");
    lost += ! _TEST_STR(conv->currency_symbol, "");
    lost += ! _TEST_STR(conv->int_curr_symbol, "");
    lost += ! _TEST_INT(conv->frac_digits, CHAR_MAX);
    lost += ! _TEST_INT(conv->p_cs_precedes, CHAR_MAX);
    lost += ! _TEST_INT(conv->n_cs_precedes, CHAR_MAX);
    lost += ! _TEST_INT(conv->p_sep_by_space, CHAR_MAX);
    lost += ! _TEST_INT(conv->n_sep_by_space, CHAR_MAX);
    lost += ! _TEST_INT(conv->p_sign_posn, CHAR_MAX);
    lost += ! _TEST_INT(conv->n_sign_posn, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_frac_digits, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_p_cs_precedes, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_n_cs_precedes, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_p_sep_by_space, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_n_sep_by_space, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_p_sign_posn, CHAR_MAX);
    lost += ! _TEST_INT(conv->int_n_sign_posn, CHAR_MAX);
    return lost == 0;
}

// (S7.11p3) The categories should be distinct constants, and NULL null.
static void Test_LocaleMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_INT(repeated(), NONE);
    _TEST_TRUE(NULL == (void *) 0);
}

// (S7.11.1.1p4) The program should start in the "C" locale in each category.
static void Test_LocaleStartup(void)
{
    _TEST_TRUE(keep(start, LC_ALL));
    _TEST_INT(refused(NULL), NONE);
    _TEST_TRUE(plain());
}

// (S7.11.1.1p6) Selecting "C" should give the name the program started with.
static void Test_LocaleSelected(void)
{
    _TEST_STR(setlocale(LC_ALL, "C"), start);
    _TEST_INT(refused("C"), NONE);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

// (S7.11.1.1p6) A locale setlocale lacks should be refused, the locale kept.
static void Test_LocaleLacking(void)
{
    _TEST_STR(setlocale(LC_ALL, LACKING), NULL);
    _TEST_STR(setlocale(LC_CTYPE, LACKING), NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

// (S7.11.1.1p8) LC_ALL's name should restore categories of two locales.
static void Test_LocaleMixed(void)
{
    char mixed[NAME_SIZE];
    char numeric[NAME_SIZE];

    _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL);
    _TEST_TRUE(keep(mixed, LC_ALL));
    _TEST_TRUE(keep(numeric, LC_NUMERIC));
    _TEST_STR(setlocale(LC_TIME, NULL), numeric);
    _TEST_TRUE(setlocale(LC_ALL, "C") != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
    _TEST_TRUE(setlocale(LC_ALL, mixed) != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), mixed);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

// (S7.11.1.1p8) The name at startup should restore locale and formatting.
static void Test_LocaleRestored(void)
{
    _TEST_INT(refused(UTF8), NONE);
    _TEST_TRUE(setlocale(LC_ALL, start) != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
    _TEST_TRUE(plain());
}

// (S7.11.2.1p8) A copy of the formatting should be the program's to change.
static void Test_LocaleFormatting(void)
{
    struct lconv copy = *localeconv();

    _TEST_STR(copy.decimal_point, ".");
    _TEST_INT(copy.frac_digits, CHAR_MAX);
    copy.decimal_point = ",";
    copy.frac_digits = 2;
    _TEST_STR(copy.decimal_point, ",");
    _TEST_INT(copy.frac_digits, 2);
    _TEST_TRUE(plain());
}

// (S7.1.4p1) A pointer and a call in parentheses should reach each function.
static void Test_LocaleCalls(void)
{
    struct lconv *(*conv)(void) = localeconv;
    char *(*set)(int, const char *) = setlocale;

    _TEST_TRUE(set(LC_TIME, "C") != NULL);
    _TEST_TRUE((setlocale)(LC_TIME, NULL) != NULL);
    _TEST_STR(set(LC_ALL, NULL), start);
    _TEST_STR((setlocale)(LC_ALL, LACKING), NULL);
    _TEST_STR(conv()->decimal_point, ".");
    _TEST_STR((localeconv)()->decimal_point, ".");
}

int main(void)
{
    Test_LocaleMacros();
    Test_LocaleStartup();
    Test_LocaleSelected();
    Test_LocaleLacking();
    Test_LocaleMixed();
    Test_LocaleRestored();
    Test_LocaleFormatting();
    Test_LocaleCalls();
    return _Test_Status();
}
