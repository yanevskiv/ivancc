/*
 * C header file for the loops over wide characters and wide strings.
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

#ifndef _WSTR_H
#define _WSTR_H

// A null wide string, and what a search gives when it finds nothing.
#define _WSTR_NULL ((int *) 0)

// Copying
int *_Wstr_CopyArray(int *to, const int *from, unsigned long n);
int *_Wstr_MoveArray(int *to, const int *from, unsigned long n);
int *_Wstr_CopyString(int *to, const int *from);
int *_Wstr_CopyChars(int *to, const int *from, unsigned long n);

// Appending
int *_Wstr_AppendString(int *to, const int *from);
int *_Wstr_AppendChars(int *to, const int *from, unsigned long n);

// Comparing
int _Wstr_CompareArrays(const int *left, const int *right, unsigned long n);
int _Wstr_CompareStrings(const int *left, const int *right);
int _Wstr_CompareChars(const int *left, const int *right, unsigned long n);

// Searching
int          *_Wstr_FindInArray(const int *str, int wc, unsigned long n);
int          *_Wstr_FindChar(const int *str, int wc);
unsigned long _Wstr_CountUnlisted(const int *str, const int *set);
int          *_Wstr_FindListed(const int *str, const int *set);
int          *_Wstr_FindLastChar(const int *str, int wc);
unsigned long _Wstr_CountListed(const int *str, const int *set);
int          *_Wstr_FindString(const int *str, const int *part);
int          *_Wstr_SplitToken(int *str, const int *set, int **next);

// Filling and measuring
int          *_Wstr_FillArray(int *str, int wc, unsigned long n);
unsigned long _Wstr_GetLength(const int *str);

#endif // _WSTR_H
