// Includable helper file for <locale.h> tests.

// Includes.
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

// Defines.
#define NONE       (-1)
#define NAME_SIZE  256
#define CATEGORIES 6
#define LACKING    "no_SUCH.locale"
#define UTF8       "C.UTF-8"

// The categories, LC_ALL first.
static const int cats[CATEGORIES] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME};

// The name of the locale the program started in.
static char start[NAME_SIZE];

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
