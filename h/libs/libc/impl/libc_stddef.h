/*
 * C header file for the common definitions.
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

#ifndef _LIBC_IMPL_STDDEF_H
#define _LIBC_IMPL_STDDEF_H

// Common definitions
typedef long _Libc_Impl_Stddef_ptrdiff_t;
typedef unsigned long _Libc_Impl_Stddef_size_t;
typedef int _Libc_Impl_Stddef_wchar_t;

#define _LIBC_IMPL_STDDEF_NULL ((void *) 0)

#define _LIBC_IMPL_STDDEF_offsetof(type, member) __builtin_offsetof(type, member)

#endif // _LIBC_IMPL_STDDEF_H
