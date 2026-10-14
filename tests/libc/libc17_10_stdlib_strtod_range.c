// (Test) Status: 0
// A test file for <stdlib.h> strtod numbers out of range.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// strtod should read all of a string as too large, of its sign.
// [C99:7.20.1.3p10]: strtod's value, or HUGE_VAL or zero and ERANGE
static bool overflows(const char *str, int sign)
{
    int lost = 0;
    int number;
    double got;
    char *stop = NULL;

    errno = 0;
    got = strtod(str, &stop);
    number = errno;
    lost += ! _TEST_TRUE(sign * got >= DBL_MAX);
    lost += ! _TEST_INT(stop - str, strlen(str));
    lost += ! _TEST_INT(number, ERANGE);
    return lost == 0;
}

// strtod should read all of a string as too small, of its sign.
// [C99:7.20.1.3p10]: strtod's value, or HUGE_VAL or zero and ERANGE
static bool underflows(const char *str, int sign)
{
    int lost = 0;
    char *stop = NULL;
    double got = strtod(str, &stop);

    lost += ! _TEST_TRUE(sign * got >= 0 && sign * got <= DBL_MIN);
    lost += ! _TEST_INT(stop - str, strlen(str));
    return lost == 0;
}

// strtod should read a number out of range as huge or tiny.
// [C99:7.20.1.3p10]: strtod's value, or HUGE_VAL or zero and ERANGE
static void Test_StdlibStrtodRange(void)
{
    _TEST_TRUE(overflows("1e400", 1));
    _TEST_TRUE(overflows("-1e400", -1));
    _TEST_TRUE(overflows("0x1p1024", 1));
    _TEST_TRUE(underflows("1e-400", 1));
    _TEST_TRUE(underflows("-1e-400", -1));
    _TEST_TRUE(underflows("0x1p-1080", 1));
}

int main(void)
{
    Test_StdlibStrtodRange();
    return _Test_Status();
}
