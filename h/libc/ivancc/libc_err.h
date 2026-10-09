/*
 * C header file for the texts a running program prints.
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

#ifndef __LIBC_ERR_H__
#define __LIBC_ERR_H__

// The error number of no error.
#define __LIBC_ERR_ERRNO_NONE 0

// strerror's texts for the error numbers.
#define __LIBC_ERR_ERRNO_SUCCESS "Success"
#define __LIBC_ERR_ERRNO_EDOM    "Numerical argument out of domain"
#define __LIBC_ERR_ERRNO_EILSEQ  "Invalid or incomplete multibyte or wide character"
#define __LIBC_ERR_ERRNO_ERANGE  "Numerical result out of range"

// The text strerror puts before an error number it does not know.
#define __LIBC_ERR_STRING_UNKNOWN "Unknown error "

// The text assert's message puts after each of its parts.
#define __LIBC_ERR_ASSERT_PROGRAM ": "
#define __LIBC_ERR_ASSERT_FILE    ":"
#define __LIBC_ERR_ASSERT_LINE    ": "
#define __LIBC_ERR_ASSERT_FUNC    ": Assertion `"
#define __LIBC_ERR_ASSERT_EXPR    "' failed.\n"

// Error numbers
const char *__libc_err_errno_text(int errnum);

#endif // __LIBC_ERR_H__
