/*
 * C source file for wide character classification and mapping utilities.
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


// Module header.
#include <wctype.h>

// The comparison of a property's name.
#include <string.h>

// The names wctype takes, in the order of _Utf8_Class.
const char *const _Wctype_ClassNames[_WCTYPE_CLASSES] = {
    "alnum", "alpha", "blank", "cntrl", "digit", "graph", "lower", "print", "punct", "space", "upper", "xdigit"
};

// The names wctrans takes, in the order of _Wctype_Mapping past its first.
const char *const _Wctype_MappingNames[_WCTYPE_MAPPINGS] = { "tolower", "toupper" };

// Tell whether wc is in class, "C" having ASCII's characters alone.
int _Wctype_IsInClass(wint_t wc, _Utf8_Class class)
{
    return (_Utf8_Enabled || wc <= _UTF8_ASCII_MAX) && _Utf8_IsInClass(wc, class);
}

// Map wc to the case mapping names, "C" having ASCII's letters alone.
wint_t _Wctype_ToCase(wint_t wc, _Wctype_Mapping mapping)
{
    if (! _Utf8_Enabled && wc > _UTF8_ASCII_MAX) {
        return wc;
    }
    switch (mapping) {
        case _WCTYPE_MAPPING_LOWER: {
            wc = _Utf8_ToLower(wc);
        } break;
        case _WCTYPE_MAPPING_UPPER: {
            wc = _Utf8_ToUpper(wc);
        } break;
        default: {
            // empty
        } break;
    }
    return wc;
}

// Tell whether wc is a letter or a digit.
int iswalnum(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_ALNUM);
}

// Tell whether wc is a letter.
int iswalpha(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_ALPHA);
}

// Tell whether wc is a blank.
int iswblank(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_BLANK);
}

// Tell whether wc is a control character.
int iswcntrl(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_CNTRL);
}

// Tell whether wc is a decimal digit.
int iswdigit(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_DIGIT);
}

// Tell whether wc is a printing character that is no white space.
int iswgraph(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_GRAPH);
}

// Tell whether wc is a lowercase letter.
int iswlower(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_LOWER);
}

// Tell whether wc is a printing character.
int iswprint(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_PRINT);
}

// Tell whether wc is a punctuation character.
int iswpunct(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_PUNCT);
}

// Tell whether wc is a white-space character.
int iswspace(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_SPACE);
}

// Tell whether wc is an uppercase letter.
int iswupper(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_UPPER);
}

// Tell whether wc is a hexadecimal digit.
int iswxdigit(wint_t wc)
{
    return _Wctype_IsInClass(wc, _UTF8_CLASS_XDIGIT);
}

// Tell whether wc is in the class desc describes, as wctype gave it.
int iswctype(wint_t wc, wctype_t desc)
{
    return desc >= 1 && desc <= _WCTYPE_CLASSES && _Wctype_IsInClass(wc, (_Utf8_Class) (desc - 1));
}

// Describe the class property names, or give 0 where it names none.
wctype_t wctype(const char *property)
{
    for (int i = 0; i < _WCTYPE_CLASSES; i++) {
        if (strcmp(property, _Wctype_ClassNames[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

// Convert an uppercase letter to lowercase.
wint_t towlower(wint_t wc)
{
    return _Wctype_ToCase(wc, _WCTYPE_MAPPING_LOWER);
}

// Convert a lowercase letter to uppercase.
wint_t towupper(wint_t wc)
{
    return _Wctype_ToCase(wc, _WCTYPE_MAPPING_UPPER);
}

// Map wc by the mapping desc describes, as wctrans gave it.
wint_t towctrans(wint_t wc, wctrans_t desc)
{
    return _Wctype_ToCase(wc, (_Wctype_Mapping) desc);
}

// Describe the mapping property names, or give 0 where it names none.
wctrans_t wctrans(const char *property)
{
    for (int i = 0; i < _WCTYPE_MAPPINGS; i++) {
        if (strcmp(property, _Wctype_MappingNames[i]) == 0) {
            return _WCTYPE_MAPPING_LOWER + i;
        }
    }
    return _WCTYPE_MAPPING_NONE;
}
