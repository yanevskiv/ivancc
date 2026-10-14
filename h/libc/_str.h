/*
 * C header file for strings and arrays of bytes.
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


#ifndef _STR_H
#define _STR_H

// A null string, and what a search gives when it finds nothing.
#define _STR_NULL ((char *) 0)

// Copying
void *_Str_CopyBytes(void *to, const void *from, unsigned long n);
void *_Str_MoveBytes(void *to, const void *from, unsigned long n);
char *_Str_CopyString(char *to, const char *from);
char *_Str_CopyChars(char *to, const char *from, unsigned long n);

// Appending
char *_Str_AppendString(char *to, const char *from);
char *_Str_AppendChars(char *to, const char *from, unsigned long n);

// Comparing
int _Str_CompareBytes(const void *left, const void *right, unsigned long n);
int _Str_CompareStrings(const char *left, const char *right);
int _Str_CompareChars(const char *left, const char *right, unsigned long n);

// Searching
void         *_Str_FindByte(const void *str, int ch, unsigned long n);
char         *_Str_FindChar(const char *str, int ch);
unsigned long _Str_CountUnlisted(const char *str, const char *set);
char         *_Str_FindListed(const char *str, const char *set);
char         *_Str_FindLastChar(const char *str, int ch);
unsigned long _Str_CountListed(const char *str, const char *set);
char         *_Str_FindString(const char *str, const char *part);
char         *_Str_SplitToken(char *str, const char *set, char **next);

// Filling and measuring
void         *_Str_FillBytes(void *str, int ch, unsigned long n);
unsigned long _Str_GetLength(const char *str);

#endif // _STR_H
