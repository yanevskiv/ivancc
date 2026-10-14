// (Test) Status: 0
// A test file for <inttypes.h> strtoimax.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99:7.8.2.3p3] strtoimax should give the number, the end and the error given.
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

// [C99:7.8.2.3p2] strtoimax should take space, a sign, a prefix and digits.
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

int main(void)
{
    Test_InttypesSigned();
    return _Test_Status();
}
