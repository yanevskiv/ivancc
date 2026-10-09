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

// Tell whether ch is a letter or a digit.
int isalnum(int ch)
{
    return isalpha(ch) || isdigit(ch);
}

// Tell whether ch is a letter.
int isalpha(int ch)
{
    return isupper(ch) || islower(ch);
}

// Tell whether ch is a blank.
int isblank(int ch)
{
    return ch == ' ' || ch == '\t';
}

// Tell whether ch is a control character.
int iscntrl(int ch)
{
    return (ch >= 0 && ch < ' ') || ch == 0x7F;
}

// Tell whether ch is a decimal digit.
int isdigit(int ch)
{
    return ch >= '0' && ch <= '9';
}

// Tell whether ch is a printing character other than space.
int isgraph(int ch)
{
    return ch > ' ' && ch < 0x7F;
}

// Tell whether ch is a lowercase letter.
int islower(int ch)
{
    return ch >= 'a' && ch <= 'z';
}

// Tell whether ch is a printing character.
int isprint(int ch)
{
    return ch >= ' ' && ch < 0x7F;
}

// Tell whether ch is a punctuation character.
int ispunct(int ch)
{
    return isgraph(ch) && ! isalnum(ch);
}

// Tell whether ch is a white-space character.
int isspace(int ch)
{
    return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

// Tell whether ch is an uppercase letter.
int isupper(int ch)
{
    return ch >= 'A' && ch <= 'Z';
}

// Tell whether ch is a hexadecimal digit.
int isxdigit(int ch)
{
    return isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

// Convert an uppercase letter to lowercase.
int tolower(int ch)
{
    if (isupper(ch)) {
        return ch - 'A' + 'a';
    }
    return ch;
}

// Convert a lowercase letter to uppercase.
int toupper(int ch)
{
    if (islower(ch)) {
        return ch - 'a' + 'A';
    }
    return ch;
}
