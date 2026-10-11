// (Test) Status: 0
// <locale.h> defines struct lconv and the categories, setlocale, which selects and names a locale, and localeconv, which gives its formatting (S7.11).

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

static char start[256];
static char mixed[256];

// Copy the name setlocale returns, which its next call may overwrite.
static bool keep(char *buf, const char *name)
{
    if (name == NULL or strlen(name) >= 256) return false;
    strcpy(buf, name);
    return true;
}

// Check the formatting (S7.11.2.1) gives the "C" locale.
static bool is_c_lconv(const struct lconv *conv)
{
    if (conv == NULL) return false;
    if (strcmp(conv->decimal_point, ".") != 0) return false;
    if (strcmp(conv->thousands_sep, "") != 0 or strcmp(conv->grouping, "") != 0) return false;
    if (strcmp(conv->mon_decimal_point, "") != 0 or strcmp(conv->mon_thousands_sep, "") != 0) return false;
    if (strcmp(conv->mon_grouping, "") != 0 or strcmp(conv->positive_sign, "") != 0) return false;
    if (strcmp(conv->negative_sign, "") != 0 or strcmp(conv->currency_symbol, "") != 0) return false;
    if (strcmp(conv->int_curr_symbol, "") != 0) return false;
    if (conv->frac_digits != CHAR_MAX or conv->p_cs_precedes != CHAR_MAX or conv->n_cs_precedes != CHAR_MAX) return false;
    if (conv->p_sep_by_space != CHAR_MAX or conv->n_sep_by_space != CHAR_MAX) return false;
    if (conv->p_sign_posn != CHAR_MAX or conv->n_sign_posn != CHAR_MAX) return false;
    if (conv->int_frac_digits != CHAR_MAX or conv->int_p_cs_precedes != CHAR_MAX or conv->int_n_cs_precedes != CHAR_MAX) return false;
    if (conv->int_p_sep_by_space != CHAR_MAX or conv->int_n_sep_by_space != CHAR_MAX) return false;
    if (conv->int_p_sign_posn != CHAR_MAX or conv->int_n_sign_posn != CHAR_MAX) return false;
    return true;
}

int main(void)
{
    static const int cats[6] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME};
    char *(*set)(int, const char *) = setlocale;
    struct lconv *(*conv)(void) = localeconv;
    struct lconv copy;
    char *name;

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < i; j++) {
            if (cats[i] == cats[j]) return 1;
        }
    }
    if (NULL != (void *) 0) return 1;

    if (not keep(start, setlocale(LC_ALL, NULL))) return 2;
    for (int i = 1; i < 6; i++) {
        if (setlocale(cats[i], NULL) == NULL) return 2;
    }
    if (not is_c_lconv(localeconv())) return 3;

    name = setlocale(LC_ALL, "C");
    if (name == NULL or strcmp(name, start) != 0) return 4;
    for (int i = 0; i < 6; i++) {
        if (setlocale(cats[i], "C") == NULL) return 4;
    }
    if (strcmp(setlocale(LC_ALL, NULL), start) != 0) return 4;

    if (setlocale(LC_ALL, "no_SUCH.locale") != NULL) return 5;
    if (setlocale(LC_CTYPE, "no_SUCH.locale") != NULL) return 5;
    if (strcmp(setlocale(LC_ALL, NULL), start) != 0) return 5;

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 6;
    if (not keep(mixed, setlocale(LC_ALL, NULL))) return 6;
    if (strcmp(setlocale(LC_NUMERIC, NULL), setlocale(LC_TIME, NULL)) != 0) return 6;
    if (setlocale(LC_ALL, "C") == NULL) return 6;
    if (setlocale(LC_ALL, mixed) == NULL) return 6;
    if (strcmp(setlocale(LC_ALL, NULL), mixed) != 0) return 6;
    if (setlocale(LC_CTYPE, "C") == NULL or strcmp(setlocale(LC_ALL, NULL), start) != 0) return 6;

    for (int i = 0; i < 6; i++) {
        if (setlocale(cats[i], "C.UTF-8") == NULL) return 7;
    }
    if (setlocale(LC_ALL, start) == NULL or strcmp(setlocale(LC_ALL, NULL), start) != 0) return 7;
    if (not is_c_lconv(localeconv())) return 7;

    copy = *localeconv();
    copy.decimal_point = ",";
    if (not is_c_lconv(localeconv())) return 8;
    if ((*conv)() == NULL or (localeconv)() == NULL) return 8;

    if ((*set)(LC_TIME, "C") == NULL or (setlocale)(LC_TIME, NULL) == NULL) return 9;

    return 0;
}
