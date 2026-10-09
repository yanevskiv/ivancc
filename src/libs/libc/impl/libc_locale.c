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
static const struct __libc_impl_locale_name __libc_impl_locale_names[__LIBC_IMPL_LOCALE_NAMES] = {
    { __LIBC_IMPL_LOCALE_NAME_C, __LIBC_IMPL_LOCALE_NAME_C },
    { __LIBC_IMPL_LOCALE_NAME_POSIX, __LIBC_IMPL_LOCALE_NAME_C },
    { __LIBC_IMPL_LOCALE_NAME_UTF8, __LIBC_IMPL_LOCALE_NAME_UTF8 },
    { __LIBC_IMPL_LOCALE_NAME_UTF8_ALT, __LIBC_IMPL_LOCALE_NAME_UTF8_ALT },
};

// The names of the categories, as a composite name writes them.
static const char *const __libc_impl_locale_categories[__LIBC_IMPL_LOCALE_CATEGORIES] = { "LC_CTYPE", "LC_NUMERIC", "LC_TIME", "LC_COLLATE", "LC_MONETARY" };

// The formatting of both locales, as glibc's "C.UTF-8" shares "C"'s.
static const struct lconv __libc_impl_locale_c_lconv = {
    ".", "", "", "", "", "", "", "", "",
    __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX,
    __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX,
    "",
    __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX,
    __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX, __LIBC_IMPL_LIMITS_CHAR_MAX,
};

// The name each category's locale was selected by, all "C" at startup.
static int __libc_impl_locale_current[__LIBC_IMPL_LOCALE_CATEGORIES];

// The formatting localeconv returns, refilled at each call.
static struct lconv __libc_impl_locale_lconv;

// The composite name setlocale returns.
static char __libc_impl_locale_composite_name[__LIBC_IMPL_LOCALE_COMPOSITE_SIZE];

// Select the locale for category, or name its locale where locale is null.
char *__libc_impl_locale_setlocale(int category, const char *locale)
{
    int selected[__LIBC_IMPL_LOCALE_CATEGORIES];
    int name;

    if (category < 0 || category > __LIBC_IMPL_LOCALE_LC_ALL) {
        return __LIBC_IMPL_STDDEF_NULL;
    }
    if (locale == __LIBC_IMPL_STDDEF_NULL) {
        return __libc_impl_locale_query(category);
    }
    if (category == __LIBC_IMPL_LOCALE_LC_ALL && __libc_impl_string_strchr(locale, __LIBC_IMPL_LOCALE_ASSIGN) != __LIBC_IMPL_STDDEF_NULL) {
        if (! __libc_impl_locale_parse(locale, selected)) {
            return __LIBC_IMPL_STDDEF_NULL;
        }
        for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
            __libc_impl_locale_current[i] = selected[i];
        }
        return __libc_impl_locale_query(category);
    }
    if (*locale == '\0') {
        locale = __LIBC_IMPL_LOCALE_NATIVE;
    }
    name = __libc_impl_locale_find_name(locale, __libc_impl_string_strlen(locale));
    if (name < 0) {
        return __LIBC_IMPL_STDDEF_NULL;
    }
    for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (category == __LIBC_IMPL_LOCALE_LC_ALL || category == i) {
            __libc_impl_locale_current[i] = name;
        }
    }
    return __libc_impl_locale_query(category);
}

// Return the numeric and monetary formatting of the current locale.
struct lconv *__libc_impl_locale_localeconv(void)
{
    __libc_impl_locale_lconv = __libc_impl_locale_c_lconv;
    return &__libc_impl_locale_lconv;
}

// Check whether the len characters of str spell the name known.
_Bool __libc_impl_locale_is_name(const char *known, const char *str, __libc_impl_stddef_size_t len)
{
    return __libc_impl_string_strlen(known) == len && __libc_impl_string_strncmp(known, str, len) == 0;
}

// Return the index of the locale name the len characters of str spell, or -1.
int __libc_impl_locale_find_name(const char *str, __libc_impl_stddef_size_t len)
{
    for (int i = 0; i < __LIBC_IMPL_LOCALE_NAMES; i++) {
        if (__libc_impl_locale_is_name(__libc_impl_locale_names[i].ln_name, str, len)) {
            return i;
        }
    }
    return -1;
}

// Return the category the len characters of str name, or -1.
int __libc_impl_locale_find_category(const char *str, __libc_impl_stddef_size_t len)
{
    for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (__libc_impl_locale_is_name(__libc_impl_locale_categories[i], str, len)) {
            return i;
        }
    }
    return -1;
}

// Read the composite name locale into selected, which must name every category.
_Bool __libc_impl_locale_parse(const char *locale, int *selected)
{
    const char *ptr = locale;

    for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        selected[i] = -1;
    }
    for (;;) {
        const char *assign = __libc_impl_string_strchr(ptr, __LIBC_IMPL_LOCALE_ASSIGN);
        const char *end;
        int category;
        int name;

        if (assign == __LIBC_IMPL_STDDEF_NULL) {
            return 0;
        }
        end = __libc_impl_string_strchr(assign, __LIBC_IMPL_LOCALE_SEPARATOR);
        if (end == __LIBC_IMPL_STDDEF_NULL) {
            end = assign + __libc_impl_string_strlen(assign);
        }
        category = __libc_impl_locale_find_category(ptr, (__libc_impl_stddef_size_t) (assign - ptr));
        name = __libc_impl_locale_find_name(assign + 1, (__libc_impl_stddef_size_t) (end - assign - 1));
        if (category < 0 || name < 0) {
            return 0;
        }
        selected[category] = name;
        if (*end == '\0') {
            break;
        }
        ptr = end + 1;
    }
    for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (selected[i] < 0) {
            return 0;
        }
    }
    return 1;
}

// Write src into str at len, and return the length after it.
__libc_impl_stddef_size_t __libc_impl_locale_put_string(char *str, __libc_impl_stddef_size_t len, const char *src)
{
    __libc_impl_stddef_size_t size = __libc_impl_string_strlen(src);

    __libc_impl_string_memcpy(str + len, src, size);
    return len + size;
}

// Return the composite name of the categories' locales, as glibc writes it.
char *__libc_impl_locale_composite(void)
{
    char *str = __libc_impl_locale_composite_name;
    __libc_impl_stddef_size_t len = 0;

    for (int i = 0; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (i > 0) {
            str[len++] = __LIBC_IMPL_LOCALE_SEPARATOR;
        }
        len = __libc_impl_locale_put_string(str, len, __libc_impl_locale_categories[i]);
        str[len++] = __LIBC_IMPL_LOCALE_ASSIGN;
        len = __libc_impl_locale_put_string(str, len, __libc_impl_locale_names[__libc_impl_locale_current[i]].ln_reported);
    }
    str[len] = '\0';
    return str;
}

// Return the name of category's locale, or of all categories' for LC_ALL.
char *__libc_impl_locale_query(int category)
{
    const char *first = __libc_impl_locale_names[__libc_impl_locale_current[0]].ln_reported;

    if (category != __LIBC_IMPL_LOCALE_LC_ALL) {
        return (char *) __libc_impl_locale_names[__libc_impl_locale_current[category]].ln_reported;
    }
    for (int i = 1; i < __LIBC_IMPL_LOCALE_CATEGORIES; i++) {
        if (__libc_impl_string_strcmp(__libc_impl_locale_names[__libc_impl_locale_current[i]].ln_reported, first) != 0) {
            return __libc_impl_locale_composite();
        }
    }
    return (char *) first;
}
