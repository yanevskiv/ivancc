/*
 * C header file for wide character classification and mapping utilities.
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


#ifndef _WCTYPE_H
#define _WCTYPE_H

// The classes a wide character may be in, and the case mappings.
#include <_utf8.h>

// The counts of the classes wctype names and of the mappings wctrans names.
#define _WCTYPE_CLASSES  12
#define _WCTYPE_MAPPINGS 2

// (S7.25.1) Introduction
#ifndef __WINT_T__
#define __WINT_T__
typedef unsigned int wint_t;
#endif

typedef int wctrans_t;
typedef int wctype_t;

#ifndef WEOF
#define WEOF 0xFFFFFFFFU
#endif

// What wctrans gives for a mapping: 0 for none, as (S7.25.3.2.2p4) asks.
typedef enum _Wctype_Mapping _Wctype_Mapping;
enum _Wctype_Mapping {
    _WCTYPE_MAPPING_NONE,  // no mapping, what a name unknown gives
    _WCTYPE_MAPPING_LOWER, // "tolower", to lower case
    _WCTYPE_MAPPING_UPPER  // "toupper", to upper case
};

// The names wctype takes, in the order of _Utf8_Class, and wctrans's in ours.
extern const char *const _Wctype_ClassNames[_WCTYPE_CLASSES];
extern const char *const _Wctype_MappingNames[_WCTYPE_MAPPINGS];

// Locale
int    _Wctype_IsInClass(wint_t wc, _Utf8_Class class);
wint_t _Wctype_ToCase(wint_t wc, _Wctype_Mapping mapping);

// (S7.25.2.1) Wide character classification functions
int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
int iswblank(wint_t wc);
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);

// (S7.25.2.2) Extensible wide character classification functions
int      iswctype(wint_t wc, wctype_t desc);
wctype_t wctype(const char *property);

// (S7.25.3.1) Wide character case mapping functions
wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

// (S7.25.3.2) Extensible wide character case mapping functions
wint_t    towctrans(wint_t wc, wctrans_t desc);
wctrans_t wctrans(const char *property);

#endif // _WCTYPE_H
