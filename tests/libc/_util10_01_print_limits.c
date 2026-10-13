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
    _Static_assert(sizeof(_Print_VaList) == sizeof(va_list), "_Print_VaList should be va_list");
    _Static_assert(sizeof(_Print_IntmaxType) == sizeof(intmax_t) && (_Print_IntmaxType) -1 < 0, "_Print_IntmaxType should be intmax_t");
    _Static_assert(sizeof(_Print_UintmaxType) == sizeof(uintmax_t) && (_Print_UintmaxType) -1 > 0, "_Print_UintmaxType should be uintmax_t");
    _Static_assert(sizeof(_Print_WcharType) == sizeof(wchar_t), "_Print_WcharType should be as wide as wchar_t");
    _Static_assert(((_Print_WcharType) -1 < 0) == ((wchar_t) -1 < 0), "_Print_WcharType should be signed as wchar_t");
    _Static_assert(sizeof(_Print_WintType) == sizeof(wchar_t), "_Print_WintType should be as wide as wchar_t");
    _Static_assert(sizeof(_Print_WintType) >= sizeof(int), "_Print_WintType should be passed unpromoted");
    _Static_assert(_PRINT_INT_MAX == INT_MAX && _PRINT_INT_MIN == INT_MIN, "_print's int limits should be limits.h's");
    _Static_assert(_PRINT_SIZE_MAX == SIZE_MAX, "_PRINT_SIZE_MAX should be SIZE_MAX");
    _Static_assert(_PRINT_BYTES_MAX == MB_LEN_MAX, "_PRINT_BYTES_MAX should be MB_LEN_MAX");
    _Static_assert(_PRINT_LDOUBLE_MANT == LDBL_MANT_DIG, "_PRINT_LDOUBLE_MANT should be LDBL_MANT_DIG");
    _Static_assert(_PRINT_LDOUBLE_EXP == LDBL_MAX_EXP, "_PRINT_LDOUBLE_EXP should be LDBL_MAX_EXP");
    _Static_assert(_PRINT_DOUBLE_MIN == DBL_MIN_EXP, "_PRINT_DOUBLE_MIN should be DBL_MIN_EXP");
    _Static_assert(sizeof(size_t) == sizeof(ptrdiff_t) && sizeof(long) == sizeof(ptrdiff_t), "%z and %t should be one width");
    _Static_assert(sizeof(uintmax_t) * CHAR_BIT <= _PRINT_DIGITS_MAX * 3, "an integer's octal digits should fit their array");
    _Static_assert(sizeof(long double) >= sizeof(unsigned long long) + sizeof(unsigned short), "a long double should hold a word and an exponent");
    _Static_assert(LDBL_MANT_DIG == sizeof(unsigned long long) * CHAR_BIT, "a long double's mantissa should fill a word");
    _Static_assert(LDBL_MANT_DIG == _PRINT_HEX_BITS * _PRINT_HEX_DIGITS, "a mantissa should be its hex digits");
    _Static_assert(DBL_MANT_DIG - 1 <= _PRINT_HEX_BITS * (_PRINT_HEX_DIGITS - 1), "a double's mantissa should fit under the first");
}

int main(void)
{
    Test_PrintLimits();
    return _Test_Status();
}
