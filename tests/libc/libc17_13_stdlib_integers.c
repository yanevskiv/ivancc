// (Test) Status: 0
// A test file for <stdlib.h> strtol and its kin.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.20.1.4p5] strtol should read a string in a base as a value, and stop.
static bool converts(const char *str, int base, long value, long end)
{
    int lost = 0;
    char *stop = NULL;
    long got = strtol(str, &stop, base);

    lost += ! _TEST_INT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99:S7.20.1.4p8] strtol and its kin should read any base, a limit past range.
static void Test_StdlibIntegers(void)
{
    char *stop = NULL;
    const char *most = "99999999999999999999";

    _TEST_TRUE(converts("  -123abc", 10, -123, 6));
    _TEST_TRUE(converts("0x1F", 0, 31, 4));
    _TEST_TRUE(converts("017", 0, 15, 3));
    _TEST_TRUE(converts("z", 36, 35, 1));
    _TEST_TRUE(converts("101", 2, 5, 3));
    _TEST_TRUE(converts("-0x10", 16, -16, 5));
    _TEST_TRUE(converts("0x", 16, 0, 1));
    _TEST_TRUE(converts("09", 0, 0, 1));
    _TEST_TRUE(converts("  +", 10, 0, 0));
    _TEST_TRUE(converts("", 0, 0, 0));

    errno = 0;
    _TEST_INT(strtol(most, &stop, 10), LONG_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(stop - most, 20);
    errno = 0;
    _TEST_INT(strtoll("-99999999999999999999", NULL, 10), LLONG_MIN);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_INT(strtol("-99999999999999999999", NULL, 0), LONG_MIN);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(strtoll("12", NULL, 0), 12);

    _TEST_UINT(strtoul("-1", NULL, 10), ULONG_MAX);
    _TEST_UINT(strtoull("-1", NULL, 10), ULLONG_MAX);
    _TEST_UINT(strtoul("0xffff", NULL, 16), 0xffff);
    errno = 0;
    _TEST_UINT(strtoull("999999999999999999999999", NULL, 10), ULLONG_MAX);
    _TEST_INT(errno, ERANGE);

    _TEST_INT(atoi("  42x"), 42);
    _TEST_INT(atol("-7"), -7);
    _TEST_INT(atoll("123456789012"), 123456789012LL);
    _TEST_FLOAT(atof("2.5x"), 2.5);
}

int main(void)
{
    Test_StdlibIntegers();
    return _Test_Status();
}
