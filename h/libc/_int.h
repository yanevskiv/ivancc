/*
 * C header file for the conversion of text to an integer.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */


#ifndef _INT_H
#define _INT_H

// The base a leading 0 selects.
#define _INT_BASE_OCTAL 8

// The base a number without a prefix is read in.
#define _INT_BASE_DECIMAL 10

// The base a leading 0x selects.
#define _INT_BASE_HEX 16

// The greatest base, whose digits run to z, and the value of a non-digit.
#define _INT_BASE_MAX 36

// The value of the digit a.
#define _INT_DIGIT_LETTER 10

// The greatest magnitude, and the greatest and least signed value.
#define _INT_VALUE_MAX  ((unsigned long) -1)
#define _INT_SIGNED_MAX ((long) (_INT_VALUE_MAX >> 1))
#define _INT_SIGNED_MIN (-_INT_SIGNED_MAX - 1)

// The characters a text to convert is made of.
typedef enum _Int_Text _Int_Text;
enum _Int_Text {
    _INT_TEXT_NARROW, // a string of char
    _INT_TEXT_WIDE    // a string of wide characters
};

// The sign a number was written with.
typedef enum _Int_Sign _Int_Sign;
enum _Int_Sign {
    _INT_SIGN_PLUS, // a plus sign, or none
    _INT_SIGN_MINUS // a minus sign
};

// Where a number's magnitude lies, of what an unsigned long holds.
typedef enum _Int_Range _Int_Range;
enum _Int_Range {
    _INT_RANGE_INSIDE, // an unsigned long holds it
    _INT_RANGE_OUTSIDE // it is greater than any unsigned long
};

// Conversion
int           _Int_ReadChar(const void *str, _Int_Text text, unsigned long index);
_Bool         _Int_IsSpace(int ch);
int           _Int_ToDigit(int ch);
unsigned long _Int_ToNumber(const void *str, _Int_Text text, unsigned long *end, int base, _Int_Sign *sign, _Int_Range *range);
long          _Int_MakeSigned(unsigned long value, _Int_Sign sign, _Int_Range range);
unsigned long _Int_MakeUnsigned(unsigned long value, _Int_Sign sign, _Int_Range range);

#endif // _INT_H
