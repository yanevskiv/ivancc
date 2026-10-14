/*
 * C header file for the Unicode tables, generated from glibc's "C.UTF-8".
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

#ifndef _UTF8_TABLE_H
#define _UTF8_TABLE_H

// The entries of each table, as glibc 2.41 has them.
#define _UTF8_TABLE_ALPHA_COUNT    801
#define _UTF8_TABLE_GRAPH_COUNT    737
#define _UTF8_TABLE_UPPER_COUNT    659
#define _UTF8_TABLE_LOWER_COUNT    675
#define _UTF8_TABLE_SPACE_COUNT    8
#define _UTF8_TABLE_BLANK_COUNT    7
#define _UTF8_TABLE_CNTRL_COUNT    3
#define _UTF8_TABLE_TO_UPPER_COUNT 205
#define _UTF8_TABLE_TO_LOWER_COUNT 187

// A range of codes, its first and its last both in it.
typedef struct _Utf8_Table_Range _Utf8_Table_Range;
struct _Utf8_Table_Range {
    unsigned int tr_first; // the first code
    unsigned int tr_last;  // the last code
};

// A shift: codes from a first to a last, a step apart, mapped a distance away.
typedef struct _Utf8_Table_Shift _Utf8_Table_Shift;
struct _Utf8_Table_Shift {
    unsigned int ts_first; // the first code
    unsigned int ts_last;  // the last code
    unsigned int ts_step;  // the distance from one code to the next
    int ts_delta;          // what a code gains by the mapping
};

// The letters.
extern const _Utf8_Table_Range _Utf8_Table_Alpha[_UTF8_TABLE_ALPHA_COUNT];

// The codes with a visible form.
extern const _Utf8_Table_Range _Utf8_Table_Graph[_UTF8_TABLE_GRAPH_COUNT];

// The upper-case letters.
extern const _Utf8_Table_Range _Utf8_Table_Upper[_UTF8_TABLE_UPPER_COUNT];

// The lower-case letters.
extern const _Utf8_Table_Range _Utf8_Table_Lower[_UTF8_TABLE_LOWER_COUNT];

// The white space.
extern const _Utf8_Table_Range _Utf8_Table_Space[_UTF8_TABLE_SPACE_COUNT];

// The blanks, the white space that separates words in a line.
extern const _Utf8_Table_Range _Utf8_Table_Blank[_UTF8_TABLE_BLANK_COUNT];

// The control codes.
extern const _Utf8_Table_Range _Utf8_Table_Cntrl[_UTF8_TABLE_CNTRL_COUNT];

// The shifts of letters to their upper case.
extern const _Utf8_Table_Shift _Utf8_Table_ToUpper[_UTF8_TABLE_TO_UPPER_COUNT];

// The shifts of letters to their lower case.
extern const _Utf8_Table_Shift _Utf8_Table_ToLower[_UTF8_TABLE_TO_LOWER_COUNT];

#endif // _UTF8_TABLE_H
