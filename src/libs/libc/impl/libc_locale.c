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
#include <libc/impl/libc_locale.h>

// The value of a char member no locale gives.
#include <libc/impl/libc_limits.h>

// The names setlocale compares and joins.
#include <libc/impl/libc_string.h>

// The names setlocale takes, each with the name it returns, as glibc's do.
static const struct _Libc_Impl_Locale_Name _Libc_Impl_Locale_Names[_LIBC_IMPL_LOCALE_NAMES] = {
    { _LIBC_IMPL_LOCALE_NAME_C, _LIBC_IMPL_LOCALE_NAME_C },
    { _LIBC_IMPL_LOCALE_NAME_POSIX, _LIBC_IMPL_LOCALE_NAME_C },
    { _LIBC_IMPL_LOCALE_NAME_UTF8, _LIBC_IMPL_LOCALE_NAME_UTF8 },
    { _LIBC_IMPL_LOCALE_NAME_UTF8_ALT, _LIBC_IMPL_LOCALE_NAME_UTF8_ALT },
};

// The names of the categories, as a composite name writes them.
static const char *const _Libc_Impl_Locale_Categories[_LIBC_IMPL_LOCALE_CATEGORIES] = { "LC_CTYPE", "LC_NUMERIC", "LC_TIME", "LC_COLLATE", "LC_MONETARY" };

// The formatting of both locales, as glibc's "C.UTF-8" shares "C"'s.
static const struct lconv _Libc_Impl_Locale_CLconv = {
    ".", "", "", "", "", "", "", "", "",
    _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX,
    _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX,
    "",
    _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX,
    _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX, _LIBC_IMPL_LIMITS_CHAR_MAX,
};

// The name each category's locale was selected by, all "C" at startup.
static int _Libc_Impl_Locale_Current[_LIBC_IMPL_LOCALE_CATEGORIES];

// The formatting localeconv returns, refilled at each call.
static struct lconv _Libc_Impl_Locale_Lconv;

// The composite name setlocale returns.
static char _Libc_Impl_Locale_CompositeName[_LIBC_IMPL_LOCALE_COMPOSITE_SIZE];

// Select the locale for category, or name its locale where locale is null.
char *_Libc_Impl_Locale_setlocale(int category, const char *locale)
{
    int selected[_LIBC_IMPL_LOCALE_CATEGORIES];
    int name;

    if (category < 0 || category > _LIBC_IMPL_LOCALE_LC_ALL) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    if (locale == _LIBC_IMPL_STDDEF_NULL) {
        return _Libc_Impl_Locale_Query(category);
    }
    if (category == _LIBC_IMPL_LOCALE_LC_ALL && _Libc_Impl_String_strchr(locale, _LIBC_IMPL_LOCALE_ASSIGN) != _LIBC_IMPL_STDDEF_NULL) {
        if (! _Libc_Impl_Locale_Parse(locale, selected)) {
            return _LIBC_IMPL_STDDEF_NULL;
        }
        for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
            _Libc_Impl_Locale_Current[i] = selected[i];
        }
        return _Libc_Impl_Locale_Query(category);
    }
    if (*locale == '\0') {
        locale = _LIBC_IMPL_LOCALE_NATIVE;
    }
    name = _Libc_Impl_Locale_FindName(locale, _Libc_Impl_String_strlen(locale));
    if (name < 0) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (category == _LIBC_IMPL_LOCALE_LC_ALL || category == i) {
            _Libc_Impl_Locale_Current[i] = name;
        }
    }
    return _Libc_Impl_Locale_Query(category);
}

// Return the numeric and monetary formatting of the current locale.
struct lconv *_Libc_Impl_Locale_localeconv(void)
{
    _Libc_Impl_Locale_Lconv = _Libc_Impl_Locale_CLconv;
    return &_Libc_Impl_Locale_Lconv;
}

// Check whether the len characters of str spell the name known.
_Bool _Libc_Impl_Locale_IsName(const char *known, const char *str, _Libc_Impl_Stddef_size_t len)
{
    return _Libc_Impl_String_strlen(known) == len && _Libc_Impl_String_strncmp(known, str, len) == 0;
}

// Return the index of the locale name the len characters of str spell, or -1.
int _Libc_Impl_Locale_FindName(const char *str, _Libc_Impl_Stddef_size_t len)
{
    for (int i = 0; i < _LIBC_IMPL_LOCALE_NAMES; i++) {
        if (_Libc_Impl_Locale_IsName(_Libc_Impl_Locale_Names[i].ln_name, str, len)) {
            return i;
        }
    }
    return -1;
}

// Return the category the len characters of str name, or -1.
int _Libc_Impl_Locale_FindCategory(const char *str, _Libc_Impl_Stddef_size_t len)
{
    for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (_Libc_Impl_Locale_IsName(_Libc_Impl_Locale_Categories[i], str, len)) {
            return i;
        }
    }
    return -1;
}

// Read the composite name locale into selected, which must name every category.
_Bool _Libc_Impl_Locale_Parse(const char *locale, int *selected)
{
    const char *ptr = locale;

    for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        selected[i] = -1;
    }
    for (;;) {
        const char *assign = _Libc_Impl_String_strchr(ptr, _LIBC_IMPL_LOCALE_ASSIGN);
        const char *end;
        int category;
        int name;

        if (assign == _LIBC_IMPL_STDDEF_NULL) {
            return 0;
        }
        end = _Libc_Impl_String_strchr(assign, _LIBC_IMPL_LOCALE_SEPARATOR);
        if (end == _LIBC_IMPL_STDDEF_NULL) {
            end = assign + _Libc_Impl_String_strlen(assign);
        }
        category = _Libc_Impl_Locale_FindCategory(ptr, (_Libc_Impl_Stddef_size_t) (assign - ptr));
        name = _Libc_Impl_Locale_FindName(assign + 1, (_Libc_Impl_Stddef_size_t) (end - assign - 1));
        if (category < 0 || name < 0) {
            return 0;
        }
        selected[category] = name;
        if (*end == '\0') {
            break;
        }
        ptr = end + 1;
    }
    for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (selected[i] < 0) {
            return 0;
        }
    }
    return 1;
}

// Write src into str at len, and return the length after it.
_Libc_Impl_Stddef_size_t _Libc_Impl_Locale_PutString(char *str, _Libc_Impl_Stddef_size_t len, const char *src)
{
    _Libc_Impl_Stddef_size_t size = _Libc_Impl_String_strlen(src);

    _Libc_Impl_String_memcpy(str + len, src, size);
    return len + size;
}

// Return the composite name of the categories' locales, as glibc writes it.
char *_Libc_Impl_Locale_Composite(void)
{
    char *str = _Libc_Impl_Locale_CompositeName;
    _Libc_Impl_Stddef_size_t len = 0;

    for (int i = 0; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (i > 0) {
            str[len++] = _LIBC_IMPL_LOCALE_SEPARATOR;
        }
        len = _Libc_Impl_Locale_PutString(str, len, _Libc_Impl_Locale_Categories[i]);
        str[len++] = _LIBC_IMPL_LOCALE_ASSIGN;
        len = _Libc_Impl_Locale_PutString(str, len, _Libc_Impl_Locale_Names[_Libc_Impl_Locale_Current[i]].ln_reported);
    }
    str[len] = '\0';
    return str;
}

// Return the name of category's locale, or of all categories' for LC_ALL.
char *_Libc_Impl_Locale_Query(int category)
{
    const char *first = _Libc_Impl_Locale_Names[_Libc_Impl_Locale_Current[0]].ln_reported;

    if (category != _LIBC_IMPL_LOCALE_LC_ALL) {
        return (char *) _Libc_Impl_Locale_Names[_Libc_Impl_Locale_Current[category]].ln_reported;
    }
    for (int i = 1; i < _LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (_Libc_Impl_String_strcmp(_Libc_Impl_Locale_Names[_Libc_Impl_Locale_Current[i]].ln_reported, first) != 0) {
            return _Libc_Impl_Locale_Composite();
        }
    }
    return (char *) first;
}
