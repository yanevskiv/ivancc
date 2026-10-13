// (Test) Status: 0
// A test file for <_print.h> types and limits against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <_print.h>

// _print's types and limits should be C99's, and its arrays hold their digits.
static void Test_PrintLimits(void)
{
    _Static_assert(sizeof(_Print_VaList) == sizeof(va_list), "_Print_VaList is va_list");
    _Static_assert(sizeof(_Print_IntmaxType) == sizeof(intmax_t) && (_Print_IntmaxType) -1 < 0, "_Print_IntmaxType is intmax_t");
    _Static_assert(sizeof(_Print_UintmaxType) == sizeof(uintmax_t) && (_Print_UintmaxType) -1 > 0, "_Print_UintmaxType is uintmax_t");
    _Static_assert(sizeof(_Print_WcharType) == sizeof(wchar_t), "_Print_WcharType is as wide as wchar_t");
    _Static_assert(((_Print_WcharType) -1 < 0) == ((wchar_t) -1 < 0), "_Print_WcharType is signed as wchar_t");
    _Static_assert(sizeof(_Print_WintType) == sizeof(wchar_t), "_Print_WintType is as wide as wchar_t");
    _Static_assert(sizeof(_Print_WintType) >= sizeof(int), "_Print_WintType is passed unpromoted");
    _Static_assert(_PRINT_INT_MAX == INT_MAX && _PRINT_INT_MIN == INT_MIN, "_print's int limits are limits.h's");
    _Static_assert(_PRINT_SIZE_MAX == SIZE_MAX, "_PRINT_SIZE_MAX is SIZE_MAX");
    _Static_assert(_PRINT_BYTES_MAX == MB_LEN_MAX, "_PRINT_BYTES_MAX is MB_LEN_MAX");
    _Static_assert(_PRINT_LDOUBLE_MANT == LDBL_MANT_DIG, "_PRINT_LDOUBLE_MANT is LDBL_MANT_DIG");
    _Static_assert(_PRINT_LDOUBLE_EXP == LDBL_MAX_EXP, "_PRINT_LDOUBLE_EXP is LDBL_MAX_EXP");
    _Static_assert(_PRINT_DOUBLE_MIN == DBL_MIN_EXP, "_PRINT_DOUBLE_MIN is DBL_MIN_EXP");
    _Static_assert(sizeof(size_t) == sizeof(ptrdiff_t) && sizeof(long) == sizeof(ptrdiff_t), "%z and %t are one width");
    _Static_assert(sizeof(uintmax_t) * CHAR_BIT <= _PRINT_DIGITS_MAX * 3, "an integer's octal digits fit their array");
    _Static_assert(sizeof(long double) >= sizeof(unsigned long long) + sizeof(unsigned short), "a long double holds a word and an exponent");
    _Static_assert(LDBL_MANT_DIG == sizeof(unsigned long long) * CHAR_BIT, "a long double's mantissa fills a word");
    _Static_assert(LDBL_MANT_DIG == _PRINT_HEX_BITS * _PRINT_HEX_DIGITS, "a mantissa is its hex digits");
    _Static_assert(DBL_MANT_DIG - 1 <= _PRINT_HEX_BITS * (_PRINT_HEX_DIGITS - 1), "a double's mantissa fits under the first");
}

int main(void)
{
    Test_PrintLimits();
    return _Test_Status();
}
