/*
 * C header file for errors.
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

#ifndef __ERRNO_H__
#define __ERRNO_H__

// The implementation, which declares errno.
#include <libc/impl/libc_errno.h>

// (S7.5) Errors
#define EDOM __LIBC_IMPL_ERRNO_EDOM
#define EILSEQ __LIBC_IMPL_ERRNO_EILSEQ
#define ERANGE __LIBC_IMPL_ERRNO_ERANGE

#endif // __ERRNO_H__
