// (Test) Status: 0
// A test file for <limits.h> limits in objects.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// (S5.2.4.2.1p1) An object of each type should hold both of its limits.
static void Test_LimitsObjects(void)
{
    int ints[2] = {INT_MIN, INT_MAX};
    char chars[2] = {CHAR_MIN, CHAR_MAX};
    long longs[2] = {LONG_MIN, LONG_MAX};
    short shorts[2] = {SHRT_MIN, SHRT_MAX};
    unsigned int uint = UINT_MAX;
    long long llongs[2] = {LLONG_MIN, LLONG_MAX};
    signed char schars[2] = {SCHAR_MIN, SCHAR_MAX};
    unsigned char uchar = UCHAR_MAX;
    unsigned long ulong = ULONG_MAX;
    unsigned short ushort = USHRT_MAX;
    unsigned long long ullong = ULLONG_MAX;

    _TEST_INT(schars[0], SCHAR_MIN);
    _TEST_INT(schars[1], SCHAR_MAX);
    _TEST_INT(uchar, UCHAR_MAX);
    _TEST_INT(chars[0], CHAR_MIN);
    _TEST_INT(chars[1], CHAR_MAX);
    _TEST_INT(shorts[0], SHRT_MIN);
    _TEST_INT(shorts[1], SHRT_MAX);
    _TEST_INT(ushort, USHRT_MAX);
    _TEST_INT(ints[0], INT_MIN);
    _TEST_INT(ints[1], INT_MAX);
    _TEST_INT(uint, UINT_MAX);
    _TEST_INT(longs[0], LONG_MIN);
    _TEST_INT(longs[1], LONG_MAX);
    _TEST_UINT(ulong, ULONG_MAX);
    _TEST_INT(llongs[0], LLONG_MIN);
    _TEST_INT(llongs[1], LLONG_MAX);
    _TEST_UINT(ullong, ULLONG_MAX);
}

int main(void)
{
    Test_LimitsObjects();
    return _Test_Status();
}
