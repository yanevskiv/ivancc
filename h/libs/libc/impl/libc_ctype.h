/*
 * C header file for character handling.
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

#ifndef _LIBC_IMPL_CTYPE_H
#define _LIBC_IMPL_CTYPE_H

// Character classification functions
int _Libc_Impl_Ctype_isalnum(int c);
int _Libc_Impl_Ctype_isalpha(int c);
int _Libc_Impl_Ctype_isblank(int c);
int _Libc_Impl_Ctype_iscntrl(int c);
int _Libc_Impl_Ctype_isdigit(int c);
int _Libc_Impl_Ctype_isgraph(int c);
int _Libc_Impl_Ctype_islower(int c);
int _Libc_Impl_Ctype_isprint(int c);
int _Libc_Impl_Ctype_ispunct(int c);
int _Libc_Impl_Ctype_isspace(int c);
int _Libc_Impl_Ctype_isupper(int c);
int _Libc_Impl_Ctype_isxdigit(int c);

// Character case mapping functions
int _Libc_Impl_Ctype_tolower(int c);
int _Libc_Impl_Ctype_toupper(int c);

#endif // _LIBC_IMPL_CTYPE_H
