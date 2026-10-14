// (Test) Status: 0
// Non-standard: a header's alias for a type it may not declare should be
// that type: wchar_t in <inttypes.h>, size_t in <locale.h>, va_list in
// <stdio.h> and <wchar.h>, whose conversion state should hold a character.
// Liberty: [C99:7.1.3p1] a name that begins with an underscore and a capital
// is reserved for any use, so a header may declare one of its own.

#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#include <_utf8.h>

_Static_assert(sizeof(_Inttypes_WcharType) == sizeof(wchar_t), "_Inttypes_WcharType should be as wide as wchar_t");
_Static_assert(((_Inttypes_WcharType) -1 < 0) == ((wchar_t) -1 < 0), "_Inttypes_WcharType should be signed as wchar_t");
_Static_assert(sizeof(_Locale_SizeType) == sizeof(size_t), "_Locale_SizeType should be as wide as size_t");
_Static_assert((_Locale_SizeType) -1 == (size_t) -1, "_Locale_SizeType should be unsigned as size_t");
_Static_assert(sizeof(_Stdio_VaList) == sizeof(va_list), "_Stdio_VaList should be va_list");
_Static_assert(sizeof(_Wchar_VaList) == sizeof(va_list), "_Wchar_VaList should be va_list");
_Static_assert(_WCHAR_STATE_BYTES == _UTF8_MAX, "mbstate_t should hold the longest character's bytes");

int main(void)
{
    return 0;
}
