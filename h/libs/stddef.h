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

#ifndef __STDDEF_H__
#define __STDDEF_H__

// The implementation.
#include <libc/impl/libc_stddef.h>

// (S7.17) Common definitions
#ifndef __PTRDIFF_T__
#define __PTRDIFF_T__
typedef _Libc_Impl_Stddef_ptrdiff_t ptrdiff_t;
#endif

#ifndef __SIZE_T__
#define __SIZE_T__
typedef _Libc_Impl_Stddef_size_t size_t;
#endif

#ifndef __WCHAR_T__
#define __WCHAR_T__
typedef _Libc_Impl_Stddef_wchar_t wchar_t;
#endif

#define NULL _LIBC_IMPL_STDDEF_NULL

#define offsetof(type, member) _LIBC_IMPL_STDDEF_offsetof(type, member)

#endif // __STDDEF_H__
