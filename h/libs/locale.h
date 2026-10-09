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

#ifndef __LOCALE_H__
#define __LOCALE_H__

// The implementation, which defines struct lconv.
#include <libc/impl/libc_locale.h>

// (S7.11) Localization
#define NULL _LIBC_IMPL_STDDEF_NULL

#define LC_ALL      _LIBC_IMPL_LOCALE_LC_ALL
#define LC_COLLATE  _LIBC_IMPL_LOCALE_LC_COLLATE
#define LC_CTYPE    _LIBC_IMPL_LOCALE_LC_CTYPE
#define LC_MONETARY _LIBC_IMPL_LOCALE_LC_MONETARY
#define LC_NUMERIC  _LIBC_IMPL_LOCALE_LC_NUMERIC
#define LC_TIME     _LIBC_IMPL_LOCALE_LC_TIME

// (S7.11.1) Locale control
char *setlocale(int category, const char *locale);

// (S7.11.2) Numeric formatting convention inquiry
struct lconv *localeconv(void);

#endif // __LOCALE_H__
