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
#include <libc/impl/libc_ctype.h>

// Tell whether c is a letter or a digit.
int _Libc_Impl_Ctype_isalnum(int c)
{
    return _Libc_Impl_Ctype_isalpha(c) || _Libc_Impl_Ctype_isdigit(c);
}

// Tell whether c is a letter.
int _Libc_Impl_Ctype_isalpha(int c)
{
    return _Libc_Impl_Ctype_isupper(c) || _Libc_Impl_Ctype_islower(c);
}

// Tell whether c is a blank.
int _Libc_Impl_Ctype_isblank(int c)
{
    return c == ' ' || c == '\t';
}

// Tell whether c is a control character.
int _Libc_Impl_Ctype_iscntrl(int c)
{
    return (c >= 0 && c < ' ') || c == 0x7F;
}

// Tell whether c is a decimal digit.
int _Libc_Impl_Ctype_isdigit(int c)
{
    return c >= '0' && c <= '9';
}

// Tell whether c is a printing character other than space.
int _Libc_Impl_Ctype_isgraph(int c)
{
    return c > ' ' && c < 0x7F;
}

// Tell whether c is a lowercase letter.
int _Libc_Impl_Ctype_islower(int c)
{
    return c >= 'a' && c <= 'z';
}

// Tell whether c is a printing character.
int _Libc_Impl_Ctype_isprint(int c)
{
    return c >= ' ' && c < 0x7F;
}

// Tell whether c is a punctuation character.
int _Libc_Impl_Ctype_ispunct(int c)
{
    return _Libc_Impl_Ctype_isgraph(c) && ! _Libc_Impl_Ctype_isalnum(c);
}

// Tell whether c is a white-space character.
int _Libc_Impl_Ctype_isspace(int c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

// Tell whether c is an uppercase letter.
int _Libc_Impl_Ctype_isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

// Tell whether c is a hexadecimal digit.
int _Libc_Impl_Ctype_isxdigit(int c)
{
    return _Libc_Impl_Ctype_isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Convert an uppercase letter to lowercase.
int _Libc_Impl_Ctype_tolower(int c)
{
    if (_Libc_Impl_Ctype_isupper(c)) {
        return c - 'A' + 'a';
    }
    return c;
}

// Convert a lowercase letter to uppercase.
int _Libc_Impl_Ctype_toupper(int c)
{
    if (_Libc_Impl_Ctype_islower(c)) {
        return c - 'a' + 'A';
    }
    return c;
}
