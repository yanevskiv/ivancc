// (Test) Status: 0
// <inttypes.h> includes <stdint.h> and defines a PRI macro for each of fprintf's conversions d, i, o, u, x and X and a SCN macro for each of fscanf's d, i, o, u and x,
// for each exact-width, least-width and fastest type and for intmax_t and intptr_t and their unsigned kin (S7.8.1), each a string literal that joins its neighbours,
// imaxdiv_t, imaxabs and imaxdiv, which truncates its quotient toward zero (S7.8.2.1-2), and strtoimax, strtoumax, wcstoimax and wcstoumax (S7.8.2.3-4),
// which skip white space, take a sign, a prefix its base allows and the digits of that base, store where they stopped, and give the type's limit and ERANGE for a number out of range,
// each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

#include <_test.h>
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

// Tell whether imaxdiv gives a quotient and a remainder.
static bool divides(intmax_t numer, intmax_t denom, intmax_t quot, intmax_t rem)
{
    int lost = 0;
    imaxdiv_t result = imaxdiv(numer, denom);

    lost += ! _TEST_INT(result.quot, quot);
    lost += ! _TEST_INT(result.rem, rem);
    return lost == 0;
}

// Tell whether strtoimax gives a number, a count of characters and an error.
static bool to_signed(const char *str, int base, intmax_t want, long used, int error)
{
    int lost = 0;
    char *end = NULL;
    int number;
    intmax_t got;

    errno = 0;
    got = strtoimax(str, &end, base);
    number = errno;
    lost += ! _TEST_INT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// Tell whether strtoumax gives a number, a count of characters and an error.
static bool to_unsigned(const char *str, int base, uintmax_t want, long used, int error)
{
    int lost = 0;
    char *end = NULL;
    int number;
    uintmax_t got;

    errno = 0;
    got = strtoumax(str, &end, base);
    number = errno;
    lost += ! _TEST_UINT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// Tell whether wcstoimax gives a number, a count of characters and an error.
static bool wide_to_signed(const wchar_t *str, int base, intmax_t want, long used, int error)
{
    int lost = 0;
    int number;
    intmax_t got;
    wchar_t *end = NULL;

    errno = 0;
    got = wcstoimax(str, &end, base);
    number = errno;
    lost += ! _TEST_INT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// Tell whether wcstoumax gives a number, a count of characters and an error.
static bool wide_to_unsigned(const wchar_t *str, int base, uintmax_t want, long used, int error)
{
    int lost = 0;
    int number;
    wchar_t *end = NULL;
    uintmax_t got;

    errno = 0;
    got = wcstoumax(str, &end, base);
    number = errno;
    lost += ! _TEST_UINT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// Each PRI macro is its type's length modifier and its conversion.
static void Test_InttypesPrint(void)
{
    _TEST_STR(PRId8, "d");
    _TEST_STR(PRId16, "d");
    _TEST_STR(PRId32, "d");
    _TEST_STR(PRId64, "ld");
    _TEST_STR(PRIdLEAST8, "d");
    _TEST_STR(PRIdLEAST16, "d");
    _TEST_STR(PRIdLEAST32, "d");
    _TEST_STR(PRIdLEAST64, "ld");
    _TEST_STR(PRIdFAST8, "d");
    _TEST_STR(PRIdFAST16, "ld");
    _TEST_STR(PRIdFAST32, "ld");
    _TEST_STR(PRIdFAST64, "ld");
    _TEST_STR(PRIdMAX, "ld");
    _TEST_STR(PRIdPTR, "ld");

    _TEST_STR(PRIi8, "i");
    _TEST_STR(PRIi16, "i");
    _TEST_STR(PRIi32, "i");
    _TEST_STR(PRIi64, "li");
    _TEST_STR(PRIiLEAST8, "i");
    _TEST_STR(PRIiLEAST16, "i");
    _TEST_STR(PRIiLEAST32, "i");
    _TEST_STR(PRIiLEAST64, "li");
    _TEST_STR(PRIiFAST8, "i");
    _TEST_STR(PRIiFAST16, "li");
    _TEST_STR(PRIiFAST32, "li");
    _TEST_STR(PRIiFAST64, "li");
    _TEST_STR(PRIiMAX, "li");
    _TEST_STR(PRIiPTR, "li");

    _TEST_STR(PRIo8, "o");
    _TEST_STR(PRIo16, "o");
    _TEST_STR(PRIo32, "o");
    _TEST_STR(PRIo64, "lo");
    _TEST_STR(PRIoLEAST8, "o");
    _TEST_STR(PRIoLEAST16, "o");
    _TEST_STR(PRIoLEAST32, "o");
    _TEST_STR(PRIoLEAST64, "lo");
    _TEST_STR(PRIoFAST8, "o");
    _TEST_STR(PRIoFAST16, "lo");
    _TEST_STR(PRIoFAST32, "lo");
    _TEST_STR(PRIoFAST64, "lo");
    _TEST_STR(PRIoMAX, "lo");
    _TEST_STR(PRIoPTR, "lo");

    _TEST_STR(PRIu8, "u");
    _TEST_STR(PRIu16, "u");
    _TEST_STR(PRIu32, "u");
    _TEST_STR(PRIu64, "lu");
    _TEST_STR(PRIuLEAST8, "u");
    _TEST_STR(PRIuLEAST16, "u");
    _TEST_STR(PRIuLEAST32, "u");
    _TEST_STR(PRIuLEAST64, "lu");
    _TEST_STR(PRIuFAST8, "u");
    _TEST_STR(PRIuFAST16, "lu");
    _TEST_STR(PRIuFAST32, "lu");
    _TEST_STR(PRIuFAST64, "lu");
    _TEST_STR(PRIuMAX, "lu");
    _TEST_STR(PRIuPTR, "lu");

    _TEST_STR(PRIx8, "x");
    _TEST_STR(PRIx16, "x");
    _TEST_STR(PRIx32, "x");
    _TEST_STR(PRIx64, "lx");
    _TEST_STR(PRIxLEAST8, "x");
    _TEST_STR(PRIxLEAST16, "x");
    _TEST_STR(PRIxLEAST32, "x");
    _TEST_STR(PRIxLEAST64, "lx");
    _TEST_STR(PRIxFAST8, "x");
    _TEST_STR(PRIxFAST16, "lx");
    _TEST_STR(PRIxFAST32, "lx");
    _TEST_STR(PRIxFAST64, "lx");
    _TEST_STR(PRIxMAX, "lx");
    _TEST_STR(PRIxPTR, "lx");

    _TEST_STR(PRIX8, "X");
    _TEST_STR(PRIX16, "X");
    _TEST_STR(PRIX32, "X");
    _TEST_STR(PRIX64, "lX");
    _TEST_STR(PRIXLEAST8, "X");
    _TEST_STR(PRIXLEAST16, "X");
    _TEST_STR(PRIXLEAST32, "X");
    _TEST_STR(PRIXLEAST64, "lX");
    _TEST_STR(PRIXFAST8, "X");
    _TEST_STR(PRIXFAST16, "lX");
    _TEST_STR(PRIXFAST32, "lX");
    _TEST_STR(PRIXFAST64, "lX");
    _TEST_STR(PRIXMAX, "lX");
    _TEST_STR(PRIXPTR, "lX");

    _TEST_STR("%" PRId64 "|%" PRIXPTR, "%ld|%lX");
}

// Each SCN macro is its type's length modifier and its conversion.
static void Test_InttypesScan(void)
{
    _TEST_STR(SCNd8, "hhd");
    _TEST_STR(SCNd16, "hd");
    _TEST_STR(SCNd32, "d");
    _TEST_STR(SCNd64, "ld");
    _TEST_STR(SCNdLEAST8, "hhd");
    _TEST_STR(SCNdLEAST16, "hd");
    _TEST_STR(SCNdLEAST32, "d");
    _TEST_STR(SCNdLEAST64, "ld");
    _TEST_STR(SCNdFAST8, "hhd");
    _TEST_STR(SCNdFAST16, "ld");
    _TEST_STR(SCNdFAST32, "ld");
    _TEST_STR(SCNdFAST64, "ld");
    _TEST_STR(SCNdMAX, "ld");
    _TEST_STR(SCNdPTR, "ld");

    _TEST_STR(SCNi8, "hhi");
    _TEST_STR(SCNi16, "hi");
    _TEST_STR(SCNi32, "i");
    _TEST_STR(SCNi64, "li");
    _TEST_STR(SCNiLEAST8, "hhi");
    _TEST_STR(SCNiLEAST16, "hi");
    _TEST_STR(SCNiLEAST32, "i");
    _TEST_STR(SCNiLEAST64, "li");
    _TEST_STR(SCNiFAST8, "hhi");
    _TEST_STR(SCNiFAST16, "li");
    _TEST_STR(SCNiFAST32, "li");
    _TEST_STR(SCNiFAST64, "li");
    _TEST_STR(SCNiMAX, "li");
    _TEST_STR(SCNiPTR, "li");

    _TEST_STR(SCNo8, "hho");
    _TEST_STR(SCNo16, "ho");
    _TEST_STR(SCNo32, "o");
    _TEST_STR(SCNo64, "lo");
    _TEST_STR(SCNoLEAST8, "hho");
    _TEST_STR(SCNoLEAST16, "ho");
    _TEST_STR(SCNoLEAST32, "o");
    _TEST_STR(SCNoLEAST64, "lo");
    _TEST_STR(SCNoFAST8, "hho");
    _TEST_STR(SCNoFAST16, "lo");
    _TEST_STR(SCNoFAST32, "lo");
    _TEST_STR(SCNoFAST64, "lo");
    _TEST_STR(SCNoMAX, "lo");
    _TEST_STR(SCNoPTR, "lo");

    _TEST_STR(SCNu8, "hhu");
    _TEST_STR(SCNu16, "hu");
    _TEST_STR(SCNu32, "u");
    _TEST_STR(SCNu64, "lu");
    _TEST_STR(SCNuLEAST8, "hhu");
    _TEST_STR(SCNuLEAST16, "hu");
    _TEST_STR(SCNuLEAST32, "u");
    _TEST_STR(SCNuLEAST64, "lu");
    _TEST_STR(SCNuFAST8, "hhu");
    _TEST_STR(SCNuFAST16, "lu");
    _TEST_STR(SCNuFAST32, "lu");
    _TEST_STR(SCNuFAST64, "lu");
    _TEST_STR(SCNuMAX, "lu");
    _TEST_STR(SCNuPTR, "lu");

    _TEST_STR(SCNx8, "hhx");
    _TEST_STR(SCNx16, "hx");
    _TEST_STR(SCNx32, "x");
    _TEST_STR(SCNx64, "lx");
    _TEST_STR(SCNxLEAST8, "hhx");
    _TEST_STR(SCNxLEAST16, "hx");
    _TEST_STR(SCNxLEAST32, "x");
    _TEST_STR(SCNxLEAST64, "lx");
    _TEST_STR(SCNxFAST8, "hhx");
    _TEST_STR(SCNxFAST16, "lx");
    _TEST_STR(SCNxFAST32, "lx");
    _TEST_STR(SCNxFAST64, "lx");
    _TEST_STR(SCNxMAX, "lx");
    _TEST_STR(SCNxPTR, "lx");

    _TEST_STR("%" SCNd8 "|%" SCNxMAX, "%hhd|%lx");
}

// imaxdiv_t holds a quotient and a remainder, each an intmax_t.
static void Test_InttypesDivision(void)
{
    imaxdiv_t result;

    result.quot = INTMAX_MAX;
    result.rem = INTMAX_MIN;
    _TEST_UINT(sizeof(result.quot), sizeof(intmax_t));
    _TEST_UINT(sizeof(result.rem), sizeof(intmax_t));
    _TEST_INT(result.quot, INTMAX_MAX);
    _TEST_INT(result.rem, INTMAX_MIN);
}

// imaxabs gives a number without its sign.
static void Test_InttypesAbsolute(void)
{
    _TEST_INT(imaxabs(0), 0);
    _TEST_INT(imaxabs(-5), 5);
    _TEST_INT(imaxabs(5), 5);
    _TEST_INT(imaxabs(INTMAX_MAX), INTMAX_MAX);
    _TEST_INT(imaxabs(-INTMAX_MAX), INTMAX_MAX);
}

// imaxdiv truncates toward zero, its remainder taking the numerator's sign.
static void Test_InttypesDivide(void)
{
    _TEST_TRUE(divides(7, 2, 3, 1));
    _TEST_TRUE(divides(-7, 2, -3, -1));
    _TEST_TRUE(divides(7, -2, -3, 1));
    _TEST_TRUE(divides(-7, -2, 3, -1));
    _TEST_TRUE(divides(0, 5, 0, 0));
    _TEST_TRUE(divides(INTMAX_MIN, 1, INTMAX_MIN, 0));
    _TEST_TRUE(divides(INTMAX_MIN, INTMAX_MAX, -1, -1));
    _TEST_TRUE(divides(INTMAX_MAX, INTMAX_MIN, 0, INTMAX_MAX));
}

// strtoimax takes white space, a sign, a prefix and digits, up to its limits.
static void Test_InttypesSigned(void)
{
    _TEST_TRUE(to_signed("0", 10, 0, 1, 0));
    _TEST_TRUE(to_signed(" \t\n\v\f\r-42xyz", 10, -42, 9, 0));
    _TEST_TRUE(to_signed("+17", 10, 17, 3, 0));
    _TEST_TRUE(to_signed("", 10, 0, 0, 0));
    _TEST_TRUE(to_signed("  ", 10, 0, 0, 0));
    _TEST_TRUE(to_signed(" +", 10, 0, 0, 0));
    _TEST_TRUE(to_signed("-", 0, 0, 0, 0));
    _TEST_TRUE(to_signed("x1", 10, 0, 0, 0));
    _TEST_TRUE(to_signed("\xA0" "1", 10, 0, 0, 0));
    _TEST_TRUE(to_signed("123", 0, 123, 3, 0));
    _TEST_TRUE(to_signed("0755", 0, 493, 4, 0));
    _TEST_TRUE(to_signed("089", 0, 0, 1, 0));
    _TEST_TRUE(to_signed("0x1F", 0, 31, 4, 0));
    _TEST_TRUE(to_signed("0X1f", 0, 31, 4, 0));
    _TEST_TRUE(to_signed("-0x10", 16, -16, 5, 0));
    _TEST_TRUE(to_signed("0x", 0, 0, 1, 0));
    _TEST_TRUE(to_signed("0x", 16, 0, 1, 0));
    _TEST_TRUE(to_signed("0xg", 16, 0, 1, 0));
    _TEST_TRUE(to_signed("0x10", 8, 0, 1, 0));
    _TEST_TRUE(to_signed("0x10", 10, 0, 1, 0));
    _TEST_TRUE(to_signed("ff", 16, 255, 2, 0));
    _TEST_TRUE(to_signed("1012", 2, 5, 3, 0));
    _TEST_TRUE(to_signed("zZ", 36, 1295, 2, 0));
    _TEST_TRUE(to_signed("Z", 35, 0, 0, 0));
    _TEST_TRUE(to_signed("9223372036854775807", 10, INTMAX_MAX, 19, 0));
    _TEST_TRUE(to_signed("9223372036854775808", 10, INTMAX_MAX, 19, ERANGE));
    _TEST_TRUE(to_signed("-9223372036854775808", 10, INTMAX_MIN, 20, 0));
    _TEST_TRUE(to_signed("-9223372036854775809", 10, INTMAX_MIN, 20, ERANGE));
    _TEST_TRUE(to_signed("99999999999999999999999", 10, INTMAX_MAX, 23, ERANGE));
    _TEST_TRUE(to_signed("-18446744073709551616", 10, INTMAX_MIN, 21, ERANGE));
    _TEST_TRUE(to_signed("0x7fffffffffffffff", 16, INTMAX_MAX, 18, 0));
    _TEST_TRUE(to_signed("-0x8000000000000000", 0, INTMAX_MIN, 19, 0));
    _TEST_INT(strtoimax("12", NULL, 10), 12);
    errno = EDOM;
    _TEST_INT(strtoimax("5", NULL, 10), 5);
    _TEST_INT(errno, EDOM);
}

// strtoumax negates a number with a minus sign, in the unsigned type.
static void Test_InttypesUnsigned(void)
{
    _TEST_TRUE(to_unsigned("18446744073709551615", 10, UINTMAX_MAX, 20, 0));
    _TEST_TRUE(to_unsigned("18446744073709551616", 10, UINTMAX_MAX, 20, ERANGE));
    _TEST_TRUE(to_unsigned("-1", 10, UINTMAX_MAX, 2, 0));
    _TEST_TRUE(to_unsigned("-18446744073709551615", 10, 1, 21, 0));
    _TEST_TRUE(to_unsigned("-18446744073709551616", 10, UINTMAX_MAX, 21, ERANGE));
    _TEST_TRUE(to_unsigned(" 0xFFFFFFFFFFFFFFFF", 0, UINTMAX_MAX, 19, 0));
    _TEST_TRUE(to_unsigned("077", 0, 63, 3, 0));
    _TEST_TRUE(to_unsigned("", 10, 0, 0, 0));
    _TEST_TRUE(to_unsigned("0x", 16, 0, 1, 0));
    _TEST_TRUE(to_unsigned("+9z", 10, 9, 2, 0));
    _TEST_UINT(strtoumax("12", NULL, 10), 12);
    errno = EDOM;
    _TEST_UINT(strtoumax("5", NULL, 10), 5);
    _TEST_INT(errno, EDOM);
}

// wcstoimax takes what strtoimax does, and no wide digit or space past ASCII.
static void Test_InttypesWideSigned(void)
{
    _TEST_TRUE(wide_to_signed(L" -0x1Fz", 0, -31, 6, 0));
    _TEST_TRUE(wide_to_signed(L"+", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"0xg", 16, 0, 1, 0));
    _TEST_TRUE(wide_to_signed(L"0755", 0, 493, 4, 0));
    _TEST_TRUE(wide_to_signed(L"9223372036854775808", 10, INTMAX_MAX, 19, ERANGE));
    _TEST_TRUE(wide_to_signed(L"-9223372036854775808", 10, INTMAX_MIN, 20, 0));
    _TEST_TRUE(wide_to_signed(L"\x10031", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"١", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"\x120" L"1", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"\xA0" L"1", 10, 0, 0, 0));
    _TEST_INT(wcstoimax(L"12", NULL, 10), 12);
}

// wcstoumax takes what strtoumax does.
static void Test_InttypesWideUnsigned(void)
{
    _TEST_TRUE(wide_to_unsigned(L"-1", 10, UINTMAX_MAX, 2, 0));
    _TEST_TRUE(wide_to_unsigned(L"zz", 36, 1295, 2, 0));
    _TEST_TRUE(wide_to_unsigned(L"18446744073709551616", 10, UINTMAX_MAX, 20, ERANGE));
    _TEST_TRUE(wide_to_unsigned(L"\t0X10", 0, 16, 5, 0));
    _TEST_TRUE(wide_to_unsigned(L"", 10, 0, 0, 0));
    _TEST_UINT(wcstoumax(L"12", NULL, 10), 12);
}

// A pointer reaches each function, and a call in parentheses any macro's.
static void Test_InttypesCalls(void)
{
    intmax_t (*absolute)(intmax_t) = imaxabs;
    imaxdiv_t (*divide)(intmax_t, intmax_t) = imaxdiv;
    intmax_t (*narrow_signed)(const char *restrict, char **restrict, int) = strtoimax;
    uintmax_t (*narrow_unsigned)(const char *restrict, char **restrict, int) = strtoumax;
    intmax_t (*wide_signed)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoimax;
    uintmax_t (*wide_unsigned)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoumax;

    _TEST_INT(absolute(-3), 3);
    _TEST_INT(divide(9, 4).quot, 2);
    _TEST_INT(divide(9, 4).rem, 1);
    _TEST_INT(narrow_signed("-42", NULL, 10), -42);
    _TEST_UINT(narrow_unsigned("42", NULL, 10), 42);
    _TEST_INT(wide_signed(L"-42", NULL, 10), -42);
    _TEST_UINT(wide_unsigned(L"42", NULL, 10), 42);

    _TEST_INT((imaxabs)(-3), 3);
    _TEST_INT((imaxdiv)(9, 4).quot, 2);
    _TEST_INT((imaxdiv)(9, 4).rem, 1);
    _TEST_INT((strtoimax)("-42", NULL, 10), -42);
    _TEST_UINT((strtoumax)("42", NULL, 10), 42);
    _TEST_INT((wcstoimax)(L"-42", NULL, 10), -42);
    _TEST_UINT((wcstoumax)(L"42", NULL, 10), 42);
}

int main(void)
{
    Test_InttypesPrint();
    Test_InttypesScan();
    Test_InttypesDivision();
    Test_InttypesAbsolute();
    Test_InttypesDivide();
    Test_InttypesSigned();
    Test_InttypesUnsigned();
    Test_InttypesWideSigned();
    Test_InttypesWideUnsigned();
    Test_InttypesCalls();
    return _Test_Status();
}
