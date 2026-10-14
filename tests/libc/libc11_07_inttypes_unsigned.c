// (Test) Status: 0
// A test file for <inttypes.h> strtoumax.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// strtoumax should give the number, the end and the error given.
// [C99:7.8.2.3p3]: strtoimax's value, or the limit and ERANGE
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

// strtoumax should negate a number with a minus sign, unsigned.
// [C99:7.8.2.3p2]: strtoimax and strtoumax, as strtol to intmax_t
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

int main(void)
{
    Test_InttypesUnsigned();
    return _Test_Status();
}
