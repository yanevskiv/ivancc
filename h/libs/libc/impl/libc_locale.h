/*
 * C header file for localization.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef __LIBC_IMPL_LOCALE_H__
#define __LIBC_IMPL_LOCALE_H__

// The sizes and null pointer the functions take.
#include <libc/impl/libc_stddef.h>

// Localization
#define __LIBC_IMPL_LOCALE_LC_ALL      5
#define __LIBC_IMPL_LOCALE_LC_COLLATE  3
#define __LIBC_IMPL_LOCALE_LC_CTYPE    0
#define __LIBC_IMPL_LOCALE_LC_MONETARY 4
#define __LIBC_IMPL_LOCALE_LC_NUMERIC  1
#define __LIBC_IMPL_LOCALE_LC_TIME     2

// The categories LC_ALL selects together, numbered from 0.
#define __LIBC_IMPL_LOCALE_CATEGORIES 5

// The names setlocale takes, "C" first, the locale a program starts in.
#define __LIBC_IMPL_LOCALE_NAME_C        "C"
#define __LIBC_IMPL_LOCALE_NAME_POSIX    "POSIX"
#define __LIBC_IMPL_LOCALE_NAME_UTF8     "C.UTF-8"
#define __LIBC_IMPL_LOCALE_NAME_UTF8_ALT "C.utf8"

// The count of the names setlocale takes.
#define __LIBC_IMPL_LOCALE_NAMES 4

// The locale "" selects, the native environment.
#define __LIBC_IMPL_LOCALE_NATIVE __LIBC_IMPL_LOCALE_NAME_UTF8

// The characters that join a composite name's categories and their locales.
#define __LIBC_IMPL_LOCALE_ASSIGN    '='
#define __LIBC_IMPL_LOCALE_SEPARATOR ';'

// The size of the longest composite name, five of "LC_MONETARY=C.UTF-8;".
#define __LIBC_IMPL_LOCALE_COMPOSITE_SIZE 128

// The numeric and monetary formatting, under C99's tag, which no typedef renames.
struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char *currency_symbol;
    char frac_digits;
    char p_cs_precedes;
    char n_cs_precedes;
    char p_sep_by_space;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    char *int_curr_symbol;
    char int_frac_digits;
    char int_p_cs_precedes;
    char int_n_cs_precedes;
    char int_p_sep_by_space;
    char int_n_sep_by_space;
    char int_p_sign_posn;
    char int_n_sign_posn;
};

// A name setlocale takes, and the name it returns for it.
struct __libc_impl_locale_name {
    const char *ln_name;
    const char *ln_reported;
};

// Locale control
char *__libc_impl_locale_setlocale(int category, const char *locale);

// Numeric formatting convention inquiry
struct lconv *__libc_impl_locale_localeconv(void);

// Names
_Bool __libc_impl_locale_is_name(const char *known, const char *str, __libc_impl_stddef_size_t len);
int __libc_impl_locale_find_name(const char *str, __libc_impl_stddef_size_t len);
int __libc_impl_locale_find_category(const char *str, __libc_impl_stddef_size_t len);

// Composite names
_Bool __libc_impl_locale_parse(const char *locale, int *selected);
__libc_impl_stddef_size_t __libc_impl_locale_put_string(char *str, __libc_impl_stddef_size_t len, const char *src);
char *__libc_impl_locale_composite(void);
char *__libc_impl_locale_query(int category);

#endif // __LIBC_IMPL_LOCALE_H__
