/*
 * C source file for character handling.
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
#include <ctype.h>

// The implementation.
#include <ivancc/impl/libc_ctype.h>

// Tell whether c is a letter or a digit.
int isalnum(int c)
{
    return __libc_impl_ctype_isalnum(c);
}

// Tell whether c is a letter.
int isalpha(int c)
{
    return __libc_impl_ctype_isalpha(c);
}

// Tell whether c is a blank.
int isblank(int c)
{
    return __libc_impl_ctype_isblank(c);
}

// Tell whether c is a control character.
int iscntrl(int c)
{
    return __libc_impl_ctype_iscntrl(c);
}

// Tell whether c is a decimal digit.
int isdigit(int c)
{
    return __libc_impl_ctype_isdigit(c);
}

// Tell whether c is a printing character other than space.
int isgraph(int c)
{
    return __libc_impl_ctype_isgraph(c);
}

// Tell whether c is a lowercase letter.
int islower(int c)
{
    return __libc_impl_ctype_islower(c);
}

// Tell whether c is a printing character.
int isprint(int c)
{
    return __libc_impl_ctype_isprint(c);
}

// Tell whether c is a punctuation character.
int ispunct(int c)
{
    return __libc_impl_ctype_ispunct(c);
}

// Tell whether c is a white-space character.
int isspace(int c)
{
    return __libc_impl_ctype_isspace(c);
}

// Tell whether c is an uppercase letter.
int isupper(int c)
{
    return __libc_impl_ctype_isupper(c);
}

// Tell whether c is a hexadecimal digit.
int isxdigit(int c)
{
    return __libc_impl_ctype_isxdigit(c);
}

// Convert an uppercase letter to lowercase.
int tolower(int c)
{
    return __libc_impl_ctype_tolower(c);
}

// Convert a lowercase letter to uppercase.
int toupper(int c)
{
    return __libc_impl_ctype_toupper(c);
}
