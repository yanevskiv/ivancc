/*
 * C source file for localization.
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

// Module header.
#include <locale.h>

// The value of a char member no locale gives.
#include <limits.h>

// The names setlocale compares and joins.
#include <string.h>

// Types the checks compare against.
#include <stddef.h>

// Check the header's own type against stddef.h's.
typedef char _Locale_CheckSize[sizeof(_Locale_SizeType) == sizeof(size_t) && (_Locale_SizeType) -1 == (size_t) -1 ? 1 : -1];

// The names setlocale takes, each with the name it returns, as glibc's do.
const struct _Locale_Name _Locale_Names[_LOCALE_NAMES] = {
    { _LOCALE_NAME_C, _LOCALE_NAME_C },
    { _LOCALE_NAME_POSIX, _LOCALE_NAME_C },
    { _LOCALE_NAME_UTF8, _LOCALE_NAME_UTF8 },
    { _LOCALE_NAME_UTF8_ALT, _LOCALE_NAME_UTF8_ALT },
};

// The names of the categories, as a composite name writes them.
const char *const _Locale_Categories[_LOCALE_CATEGORIES] = { "LC_CTYPE", "LC_NUMERIC", "LC_TIME", "LC_COLLATE", "LC_MONETARY" };

// The formatting of both locales, as glibc's "C.UTF-8" shares "C"'s.
const struct lconv _Locale_CLconv = {
    ".", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX,
    "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX,
};

// The name each category's locale was selected by, all "C" at startup.
int _Locale_Current[_LOCALE_CATEGORIES];

// The formatting localeconv returns, refilled at each call.
struct lconv _Locale_Lconv;

// The composite name setlocale returns.
char _Locale_CompositeName[_LOCALE_COMPOSITE_SIZE];

// Check whether the len characters of str spell the name known.
_Bool _Locale_IsName(const char *known, const char *str, _Locale_SizeType len)
{
    return strlen(known) == len && strncmp(known, str, len) == 0;
}

// Return the index of the locale name the len characters of str spell, or -1.
int _Locale_FindName(const char *str, _Locale_SizeType len)
{
    for (int i = 0; i < _LOCALE_NAMES; i++) {
        if (_Locale_IsName(_Locale_Names[i].ln_name, str, len)) {
            return i;
        }
    }
    return -1;
}

// Return the category the len characters of str name, or -1.
int _Locale_FindCategory(const char *str, _Locale_SizeType len)
{
    for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
        if (_Locale_IsName(_Locale_Categories[i], str, len)) {
            return i;
        }
    }
    return -1;
}

// Read the composite name locale into selected, which must name every category.
_Bool _Locale_Parse(const char *locale, int *selected)
{
    const char *ptr = locale;

    for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
        selected[i] = -1;
    }
    for (;;) {
        const char *assign = strchr(ptr, _LOCALE_ASSIGN);
        const char *end;
        int category;
        int name;

        if (assign == NULL) {
            return 0;
        }
        end = strchr(assign, _LOCALE_SEPARATOR);
        if (end == NULL) {
            end = assign + strlen(assign);
        }
        category = _Locale_FindCategory(ptr, (_Locale_SizeType) (assign - ptr));
        name = _Locale_FindName(assign + 1, (_Locale_SizeType) (end - assign - 1));
        if (category < 0 || name < 0) {
            return 0;
        }
        selected[category] = name;
        if (*end == '\0') {
            break;
        }
        ptr = end + 1;
    }
    for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
        if (selected[i] < 0) {
            return 0;
        }
    }
    return 1;
}

// Write src into str at len, and return the length after it.
_Locale_SizeType _Locale_PutString(char *str, _Locale_SizeType len, const char *src)
{
    _Locale_SizeType size = strlen(src);

    memcpy(str + len, src, size);
    return len + size;
}

// Return the composite name of the categories' locales, as glibc writes it.
char *_Locale_Composite(void)
{
    char *str = _Locale_CompositeName;
    _Locale_SizeType len = 0;

    for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
        if (i > 0) {
            str[len++] = _LOCALE_SEPARATOR;
        }
        len = _Locale_PutString(str, len, _Locale_Categories[i]);
        str[len++] = _LOCALE_ASSIGN;
        len = _Locale_PutString(str, len, _Locale_Names[_Locale_Current[i]].ln_reported);
    }
    str[len] = '\0';
    return str;
}

// Return the name of category's locale, or of all categories' for LC_ALL.
char *_Locale_Query(int category)
{
    const char *first = _Locale_Names[_Locale_Current[0]].ln_reported;

    if (category != LC_ALL) {
        return (char *) _Locale_Names[_Locale_Current[category]].ln_reported;
    }
    for (int i = 1; i < _LOCALE_CATEGORIES; i++) {
        if (strcmp(_Locale_Names[_Locale_Current[i]].ln_reported, first) != 0) {
            return _Locale_Composite();
        }
    }
    return (char *) first;
}

// Select the locale for category, or name its locale where locale is null.
char *setlocale(int category, const char *locale)
{
    int selected[_LOCALE_CATEGORIES];
    int name;

    if (category < 0 || category > LC_ALL) {
        return NULL;
    }
    if (locale == NULL) {
        return _Locale_Query(category);
    }
    if (category == LC_ALL && strchr(locale, _LOCALE_ASSIGN) != NULL) {
        if (! _Locale_Parse(locale, selected)) {
            return NULL;
        }
        for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
            _Locale_Current[i] = selected[i];
        }
        return _Locale_Query(category);
    }
    if (*locale == '\0') {
        locale = _LOCALE_NATIVE;
    }
    name = _Locale_FindName(locale, strlen(locale));
    if (name < 0) {
        return NULL;
    }
    for (int i = 0; i < _LOCALE_CATEGORIES; i++) {
        if (category == LC_ALL || category == i) {
            _Locale_Current[i] = name;
        }
    }
    return _Locale_Query(category);
}

// Return the numeric and monetary formatting of the current locale.
struct lconv *localeconv(void)
{
    _Locale_Lconv = _Locale_CLconv;
    return &_Locale_Lconv;
}
