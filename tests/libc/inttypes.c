// (Test) Status: 0
// <inttypes.h> declares the format specifier macros and the functions for greatest-width integer types (S7.8).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

static bool all_match(const char *const (*table)[2], size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (strcmp(table[i][0], table[i][1]) != 0) return false;
    }
    return true;
}

static bool to_signed(const char *str, int base, intmax_t want, size_t used, int error)
{
    char *end = NULL;
    intmax_t got;

    errno = 0;
    got = strtoimax(str, &end, base);
    return got == want and (size_t) (end - str) == used and errno == error;
}

static bool to_unsigned(const char *str, int base, uintmax_t want, size_t used, int error)
{
    char *end = NULL;
    uintmax_t got;

    errno = 0;
    got = strtoumax(str, &end, base);
    return got == want and (size_t) (end - str) == used and errno == error;
}

static bool wide_to_signed(const wchar_t *str, int base, intmax_t want, size_t used, int error)
{
    wchar_t *end = NULL;
    intmax_t got;

    errno = 0;
    got = wcstoimax(str, &end, base);
    return got == want and (size_t) (end - str) == used and errno == error;
}

static bool wide_to_unsigned(const wchar_t *str, int base, uintmax_t want, size_t used, int error)
{
    wchar_t *end = NULL;
    uintmax_t got;

    errno = 0;
    got = wcstoumax(str, &end, base);
    return got == want and (size_t) (end - str) == used and errno == error;
}

int main(void)
{
    static const char *const pri[][2] = {
        { PRId8, "d" },
        { PRId16, "d" },
        { PRId32, "d" },
        { PRId64, "ld" },
        { PRIdLEAST8, "d" },
        { PRIdLEAST16, "d" },
        { PRIdLEAST32, "d" },
        { PRIdLEAST64, "ld" },
        { PRIdFAST8, "d" },
        { PRIdFAST16, "ld" },
        { PRIdFAST32, "ld" },
        { PRIdFAST64, "ld" },
        { PRIdMAX, "ld" },
        { PRIdPTR, "ld" },
        { PRIi8, "i" },
        { PRIi16, "i" },
        { PRIi32, "i" },
        { PRIi64, "li" },
        { PRIiLEAST8, "i" },
        { PRIiLEAST16, "i" },
        { PRIiLEAST32, "i" },
        { PRIiLEAST64, "li" },
        { PRIiFAST8, "i" },
        { PRIiFAST16, "li" },
        { PRIiFAST32, "li" },
        { PRIiFAST64, "li" },
        { PRIiMAX, "li" },
        { PRIiPTR, "li" },
        { PRIo8, "o" },
        { PRIo16, "o" },
        { PRIo32, "o" },
        { PRIo64, "lo" },
        { PRIoLEAST8, "o" },
        { PRIoLEAST16, "o" },
        { PRIoLEAST32, "o" },
        { PRIoLEAST64, "lo" },
        { PRIoFAST8, "o" },
        { PRIoFAST16, "lo" },
        { PRIoFAST32, "lo" },
        { PRIoFAST64, "lo" },
        { PRIoMAX, "lo" },
        { PRIoPTR, "lo" },
        { PRIu8, "u" },
        { PRIu16, "u" },
        { PRIu32, "u" },
        { PRIu64, "lu" },
        { PRIuLEAST8, "u" },
        { PRIuLEAST16, "u" },
        { PRIuLEAST32, "u" },
        { PRIuLEAST64, "lu" },
        { PRIuFAST8, "u" },
        { PRIuFAST16, "lu" },
        { PRIuFAST32, "lu" },
        { PRIuFAST64, "lu" },
        { PRIuMAX, "lu" },
        { PRIuPTR, "lu" },
        { PRIx8, "x" },
        { PRIx16, "x" },
        { PRIx32, "x" },
        { PRIx64, "lx" },
        { PRIxLEAST8, "x" },
        { PRIxLEAST16, "x" },
        { PRIxLEAST32, "x" },
        { PRIxLEAST64, "lx" },
        { PRIxFAST8, "x" },
        { PRIxFAST16, "lx" },
        { PRIxFAST32, "lx" },
        { PRIxFAST64, "lx" },
        { PRIxMAX, "lx" },
        { PRIxPTR, "lx" },
        { PRIX8, "X" },
        { PRIX16, "X" },
        { PRIX32, "X" },
        { PRIX64, "lX" },
        { PRIXLEAST8, "X" },
        { PRIXLEAST16, "X" },
        { PRIXLEAST32, "X" },
        { PRIXLEAST64, "lX" },
        { PRIXFAST8, "X" },
        { PRIXFAST16, "lX" },
        { PRIXFAST32, "lX" },
        { PRIXFAST64, "lX" },
        { PRIXMAX, "lX" },
        { PRIXPTR, "lX" },
    };
    static const char *const scn[][2] = {
        { SCNd8, "hhd" },
        { SCNd16, "hd" },
        { SCNd32, "d" },
        { SCNd64, "ld" },
        { SCNdLEAST8, "hhd" },
        { SCNdLEAST16, "hd" },
        { SCNdLEAST32, "d" },
        { SCNdLEAST64, "ld" },
        { SCNdFAST8, "hhd" },
        { SCNdFAST16, "ld" },
        { SCNdFAST32, "ld" },
        { SCNdFAST64, "ld" },
        { SCNdMAX, "ld" },
        { SCNdPTR, "ld" },
        { SCNi8, "hhi" },
        { SCNi16, "hi" },
        { SCNi32, "i" },
        { SCNi64, "li" },
        { SCNiLEAST8, "hhi" },
        { SCNiLEAST16, "hi" },
        { SCNiLEAST32, "i" },
        { SCNiLEAST64, "li" },
        { SCNiFAST8, "hhi" },
        { SCNiFAST16, "li" },
        { SCNiFAST32, "li" },
        { SCNiFAST64, "li" },
        { SCNiMAX, "li" },
        { SCNiPTR, "li" },
        { SCNo8, "hho" },
        { SCNo16, "ho" },
        { SCNo32, "o" },
        { SCNo64, "lo" },
        { SCNoLEAST8, "hho" },
        { SCNoLEAST16, "ho" },
        { SCNoLEAST32, "o" },
        { SCNoLEAST64, "lo" },
        { SCNoFAST8, "hho" },
        { SCNoFAST16, "lo" },
        { SCNoFAST32, "lo" },
        { SCNoFAST64, "lo" },
        { SCNoMAX, "lo" },
        { SCNoPTR, "lo" },
        { SCNu8, "hhu" },
        { SCNu16, "hu" },
        { SCNu32, "u" },
        { SCNu64, "lu" },
        { SCNuLEAST8, "hhu" },
        { SCNuLEAST16, "hu" },
        { SCNuLEAST32, "u" },
        { SCNuLEAST64, "lu" },
        { SCNuFAST8, "hhu" },
        { SCNuFAST16, "lu" },
        { SCNuFAST32, "lu" },
        { SCNuFAST64, "lu" },
        { SCNuMAX, "lu" },
        { SCNuPTR, "lu" },
        { SCNx8, "hhx" },
        { SCNx16, "hx" },
        { SCNx32, "x" },
        { SCNx64, "lx" },
        { SCNxLEAST8, "hhx" },
        { SCNxLEAST16, "hx" },
        { SCNxLEAST32, "x" },
        { SCNxLEAST64, "lx" },
        { SCNxFAST8, "hhx" },
        { SCNxFAST16, "lx" },
        { SCNxFAST32, "lx" },
        { SCNxFAST64, "lx" },
        { SCNxMAX, "lx" },
        { SCNxPTR, "lx" },
    };
    intmax_t (*absolute)(intmax_t) = imaxabs;
    imaxdiv_t (*divide)(intmax_t, intmax_t) = imaxdiv;
    intmax_t (*narrow)(const char *restrict, char **restrict, int) = strtoimax;
    uintmax_t (*wide)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoumax;
    imaxdiv_t result;

    if (not all_match(pri, sizeof(pri) / sizeof(pri[0]))) return 1;
    if (strcmp("%" PRId64 "|%" PRIXPTR, "%ld|%lX") != 0) return 1;

    if (not all_match(scn, sizeof(scn) / sizeof(scn[0]))) return 2;
    if (strcmp("%" SCNd8 "|%" SCNxMAX, "%hhd|%lx") != 0) return 2;

    result.quot = INTMAX_MAX;
    result.rem = INTMAX_MIN;
    if (sizeof(result.quot) != sizeof(intmax_t) or sizeof(result.rem) != sizeof(intmax_t)) return 3;
    if (result.quot != INTMAX_MAX or result.rem != INTMAX_MIN) return 3;

    if (imaxabs(0) != 0 or imaxabs(-5) != 5 or imaxabs(5) != 5) return 4;
    if (imaxabs(INTMAX_MAX) != INTMAX_MAX or imaxabs(-INTMAX_MAX) != INTMAX_MAX) return 4;

    result = imaxdiv(7, 2);
    if (result.quot != 3 or result.rem != 1) return 5;
    result = imaxdiv(-7, 2);
    if (result.quot != -3 or result.rem != -1) return 5;
    result = imaxdiv(7, -2);
    if (result.quot != -3 or result.rem != 1) return 5;
    result = imaxdiv(INTMAX_MIN, 1);
    if (result.quot != INTMAX_MIN or result.rem != 0) return 5;
    result = imaxdiv(INTMAX_MIN, INTMAX_MAX);
    if (result.quot != -1 or result.rem != -1) return 5;

    if (not to_signed("0", 10, 0, 1, 0)) return 6;
    if (not to_signed(" \t\n\v\f\r-42xyz", 10, -42, 9, 0)) return 6;
    if (not to_signed("+17", 10, 17, 3, 0)) return 6;
    if (not to_signed("", 10, 0, 0, 0) or not to_signed("  ", 10, 0, 0, 0)) return 6;
    if (not to_signed(" +", 10, 0, 0, 0) or not to_signed("-", 0, 0, 0, 0) or not to_signed("x1", 10, 0, 0, 0)) return 6;
    if (not to_signed("\xA0" "1", 10, 0, 0, 0)) return 6;
    if (not to_signed("123", 0, 123, 3, 0) or not to_signed("0755", 0, 493, 4, 0) or not to_signed("089", 0, 0, 1, 0)) return 6;
    if (not to_signed("0x1F", 0, 31, 4, 0) or not to_signed("0X1f", 0, 31, 4, 0) or not to_signed("-0x10", 16, -16, 5, 0)) return 6;
    if (not to_signed("0x", 0, 0, 1, 0) or not to_signed("0x", 16, 0, 1, 0) or not to_signed("0xg", 16, 0, 1, 0)) return 6;
    if (not to_signed("0x10", 8, 0, 1, 0) or not to_signed("0x10", 10, 0, 1, 0) or not to_signed("ff", 16, 255, 2, 0)) return 6;
    if (not to_signed("1012", 2, 5, 3, 0) or not to_signed("zZ", 36, 1295, 2, 0) or not to_signed("Z", 35, 0, 0, 0)) return 6;
    if (not to_signed("9223372036854775807", 10, INTMAX_MAX, 19, 0)) return 6;
    if (not to_signed("9223372036854775808", 10, INTMAX_MAX, 19, ERANGE)) return 6;
    if (not to_signed("-9223372036854775808", 10, INTMAX_MIN, 20, 0)) return 6;
    if (not to_signed("-9223372036854775809", 10, INTMAX_MIN, 20, ERANGE)) return 6;
    if (not to_signed("99999999999999999999999", 10, INTMAX_MAX, 23, ERANGE)) return 6;
    if (not to_signed("-18446744073709551616", 10, INTMAX_MIN, 21, ERANGE)) return 6;
    if (not to_signed("0x7fffffffffffffff", 16, INTMAX_MAX, 18, 0)) return 6;
    if (strtoimax("12", NULL, 10) != 12) return 6;
    errno = EDOM;
    if (strtoimax("5", NULL, 10) != 5 or errno != EDOM) return 6;

    if (not to_unsigned("18446744073709551615", 10, UINTMAX_MAX, 20, 0)) return 7;
    if (not to_unsigned("18446744073709551616", 10, UINTMAX_MAX, 20, ERANGE)) return 7;
    if (not to_unsigned("-1", 10, UINTMAX_MAX, 2, 0) or not to_unsigned("-18446744073709551615", 10, 1, 21, 0)) return 7;
    if (not to_unsigned("-18446744073709551616", 10, UINTMAX_MAX, 21, ERANGE)) return 7;
    if (not to_unsigned(" 0xFFFFFFFFFFFFFFFF", 0, UINTMAX_MAX, 19, 0) or not to_unsigned("077", 0, 63, 3, 0)) return 7;
    if (not to_unsigned("", 10, 0, 0, 0) or not to_unsigned("0x", 16, 0, 1, 0) or not to_unsigned("+9z", 10, 9, 2, 0)) return 7;
    if (strtoumax("12", NULL, 10) != 12) return 7;
    errno = EDOM;
    if (strtoumax("5", NULL, 10) != 5 or errno != EDOM) return 7;

    if (not wide_to_signed(L" -0x1Fz", 0, -31, 6, 0) or not wide_to_signed(L"+", 10, 0, 0, 0)) return 8;
    if (not wide_to_signed(L"0xg", 16, 0, 1, 0) or not wide_to_signed(L"0755", 0, 493, 4, 0)) return 8;
    if (not wide_to_signed(L"9223372036854775808", 10, INTMAX_MAX, 19, ERANGE)) return 8;
    if (not wide_to_signed(L"-9223372036854775808", 10, INTMAX_MIN, 20, 0)) return 8;
    if (not wide_to_signed(L"\x10031", 10, 0, 0, 0) or not wide_to_signed(L"١", 10, 0, 0, 0)) return 8;
    if (not wide_to_signed(L"\x120" L"1", 10, 0, 0, 0) or not wide_to_signed(L"\xA0" L"1", 10, 0, 0, 0)) return 8;
    if (wcstoimax(L"12", NULL, 10) != 12) return 8;

    if (not wide_to_unsigned(L"-1", 10, UINTMAX_MAX, 2, 0) or not wide_to_unsigned(L"zz", 36, 1295, 2, 0)) return 9;
    if (not wide_to_unsigned(L"18446744073709551616", 10, UINTMAX_MAX, 20, ERANGE)) return 9;
    if (not wide_to_unsigned(L"\t0X10", 0, 16, 5, 0) or not wide_to_unsigned(L"", 10, 0, 0, 0)) return 9;
    if (wcstoumax(L"12", NULL, 10) != 12) return 9;

    if (absolute(-3) != 3 or divide(9, 4).rem != 1 or narrow("42", NULL, 10) != 42 or wide(L"42", NULL, 10) != 42) return 10;
    if ((imaxabs)(-3) != 3 or (imaxdiv)(9, 4).quot != 2 or (strtoumax)("42", NULL, 10) != 42 or (wcstoimax)(L"-42", NULL, 10) != -42) return 10;
    return 0;
}
