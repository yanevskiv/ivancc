/*
 * C source file for errors.
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
#include <errno.h>

// Linux's error numbers.
#include <_sys.h>

// Check that C99's error numbers are Linux's.
typedef char _Errno_CheckEdom[EDOM == _SYS_EDOM ? 1 : -1];
typedef char _Errno_CheckEilseq[EILSEQ == _SYS_EILSEQ ? 1 : -1];
typedef char _Errno_CheckErange[ERANGE == _SYS_ERANGE ? 1 : -1];

// The error number of the last failed call.
int errno;
