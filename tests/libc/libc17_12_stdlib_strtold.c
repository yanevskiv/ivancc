// (Test) Status: 0
// A test file for <stdlib.h> strtold.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99/S7.20.1.3p4] strtold should read a string as a value, bit for bit, and stop.
static bool reads_long(const char *str, long double value, long end)
{
    int lost = 0;
    char *stop = NULL;
    long double got = strtold(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99/S7.20.1.3p10] strtold should read to the nearest long double, to its limits.
static void Test_StdlibStrtold(void)
{
    _TEST_TRUE(reads_long("1.5", 1.5L, 3));
    _TEST_TRUE(reads_long("0.1", 0.1L, 3));
    _TEST_TRUE(reads_long("-0x1p-16382", -LDBL_MIN, 11));
    _TEST_TRUE(reads_long("0x1.fffffffffffffffep16383", LDBL_MAX, 26));
    _TEST_TRUE(reads_long("3.36210314311209350626e-4932", LDBL_MIN, 28));
    errno = 0;
    _TEST_TRUE(strtold("1e5000", NULL) > LDBL_MAX);
    _TEST_INT(errno, ERANGE);
}

int main(void)
{
    Test_StdlibStrtold();
    return _Test_Status();
}
