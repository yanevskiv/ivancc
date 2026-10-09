/*
 * C source file for the texts a running program prints.
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
#include <libc/libc_err.h>

// The error numbers the texts name.
#include <libc/impl/libc_errno.h>

// The null pointer.
#include <libc/impl/libc_stddef.h>

// Return strerror's text for the error number errnum.
const char *_Libc_Err_ErrnoText(int errnum)
{
    switch (errnum) {
        case _LIBC_ERR_ERRNO_NONE: {
            return _LIBC_ERR_ERRNO_SUCCESS;
        } break;
        case _LIBC_IMPL_ERRNO_EDOM: {
            return _LIBC_ERR_ERRNO_EDOM;
        } break;
        case _LIBC_IMPL_ERRNO_EILSEQ: {
            return _LIBC_ERR_ERRNO_EILSEQ;
        } break;
        case _LIBC_IMPL_ERRNO_ERANGE: {
            return _LIBC_ERR_ERRNO_ERANGE;
        } break;
        default: {
            return _LIBC_IMPL_STDDEF_NULL;
        } break;
    }
}
