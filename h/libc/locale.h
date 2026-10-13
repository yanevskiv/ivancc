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

#ifndef _LOCALE_H
#define _LOCALE_H

// The categories LC_ALL selects together, numbered from 0.
#define _LOCALE_CATEGORIES 5

// The names setlocale takes, "C" first, the locale a program starts in.
#define _LOCALE_NAME_C        "C"
#define _LOCALE_NAME_POSIX    "POSIX"
#define _LOCALE_NAME_UTF8     "C.UTF-8"
#define _LOCALE_NAME_UTF8_ALT "C.utf8"

// The count of the names setlocale takes.
#define _LOCALE_NAMES 4

// The locale "" selects, the native environment.
#define _LOCALE_NATIVE _LOCALE_NAME_UTF8

// The characters that join a composite name's categories and their locales.
#define _LOCALE_ASSIGN    '='
#define _LOCALE_SEPARATOR ';'

// The size of the longest composite name, five of "LC_MONETARY=C.UTF-8;".
#define _LOCALE_COMPOSITE_SIZE 128

// [C99/S7.11] Localization
#define NULL ((void *) 0)

#define LC_ALL      5
#define LC_COLLATE  3
#define LC_CTYPE    0
#define LC_MONETARY 4
#define LC_NUMERIC  1
#define LC_TIME     2

// The size, which locale.h may not name.
typedef unsigned long _Locale_SizeType;

// A name setlocale takes, the name it returns for it and its encoding.
typedef struct _Locale_Name _Locale_Name;
struct _Locale_Name {
    const char *ln_name;     // the name setlocale takes
    const char *ln_reported; // the name setlocale returns for it
    _Bool ln_utf8;           // UTF-8, not "C"'s ASCII
};

// [C99/S7.11] Localization
struct lconv {
    char *decimal_point;     // the decimal point of a number
    char *thousands_sep;     // what separates a number's digit groups
    char *grouping;          // the sizes of a number's digit groups
    char *mon_decimal_point; // the decimal point of money
    char *mon_thousands_sep; // what separates money's digit groups
    char *mon_grouping;      // the sizes of money's digit groups
    char *positive_sign;     // the sign of money not below zero
    char *negative_sign;     // the sign of money below zero
    char *currency_symbol;   // the local currency symbol
    char frac_digits;        // the digits after money's decimal point
    char p_cs_precedes;      // 1 if the symbol precedes money not below zero
    char n_cs_precedes;      // 1 if the symbol precedes money below zero
    char p_sep_by_space;     // how symbol, sign and money not below zero part
    char n_sep_by_space;     // how symbol, sign and money below zero part
    char p_sign_posn;        // where the sign of money not below zero goes
    char n_sign_posn;        // where the sign of money below zero goes
    char *int_curr_symbol;   // the international currency symbol
    char int_frac_digits;    // frac_digits, in the international format
    char int_p_cs_precedes;  // p_cs_precedes, in the international format
    char int_n_cs_precedes;  // n_cs_precedes, in the international format
    char int_p_sep_by_space; // p_sep_by_space, in the international format
    char int_n_sep_by_space; // n_sep_by_space, in the international format
    char int_p_sign_posn;    // p_sign_posn, in the international format
    char int_n_sign_posn;    // n_sign_posn, in the international format
};

// The names setlocale takes, each with the name it returns, as glibc's do.
extern const _Locale_Name _Locale_Names[_LOCALE_NAMES];

// The names of the categories, as a composite name writes them.
extern const char *const _Locale_Categories[_LOCALE_CATEGORIES];

// The formatting of both locales, as glibc's "C.UTF-8" shares "C"'s.
extern const struct lconv _Locale_CLconv;

// The name each category's locale was selected by, all "C" at startup.
extern int _Locale_Current[_LOCALE_CATEGORIES];

// The formatting localeconv returns, refilled at each call.
extern struct lconv _Locale_Lconv;

// The composite name setlocale returns.
extern char _Locale_CompositeName[_LOCALE_COMPOSITE_SIZE];

// Names
_Bool _Locale_IsName(const char *known, const char *str, _Locale_SizeType len);
int   _Locale_FindName(const char *str, _Locale_SizeType len);
int   _Locale_FindCategory(const char *str, _Locale_SizeType len);

// Composite names
_Bool            _Locale_ParseComposite(const char *locale, int *selected);
_Locale_SizeType _Locale_PutString(char *str, _Locale_SizeType len, const char *src);
char            *_Locale_FormatComposite(void);
char            *_Locale_QueryCategory(int category);

// Encoding
void _Locale_ApplyCtype(void);

// [C99/S7.11.1] Locale control
char *setlocale(int category, const char *locale);

// [C99/S7.11.2] Numeric formatting convention inquiry
struct lconv *localeconv(void);

#endif // _LOCALE_H
