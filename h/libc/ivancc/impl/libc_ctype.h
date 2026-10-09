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

#ifndef __LIBC_CTYPE_H__
#define __LIBC_CTYPE_H__

// (S7.4.1) Character classification functions
int __libc_ctype_isalnum(int c);
int __libc_ctype_isalpha(int c);
int __libc_ctype_isblank(int c);
int __libc_ctype_iscntrl(int c);
int __libc_ctype_isdigit(int c);
int __libc_ctype_isgraph(int c);
int __libc_ctype_islower(int c);
int __libc_ctype_isprint(int c);
int __libc_ctype_ispunct(int c);
int __libc_ctype_isspace(int c);
int __libc_ctype_isupper(int c);
int __libc_ctype_isxdigit(int c);

// (S7.4.2) Character case mapping functions
int __libc_ctype_tolower(int c);
int __libc_ctype_toupper(int c);

#endif // __LIBC_CTYPE_H__
