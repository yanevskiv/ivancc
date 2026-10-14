// (Test) Status: 0
// A test file for <stdlib.h> strtof.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:7.20.1.3p4] strtof should read a string as a value, bit for bit, and stop.
static bool reads_float(const char *str, float value, long end)
{
    int lost = 0;
    char *stop = NULL;
    float got = strtof(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99:7.20.1.3p10] strtof should read to the nearest float, and range as strtod.
static void Test_StdlibStrtof(void)
{
    _TEST_TRUE(reads_float("1.5", 1.5F, 3));
    _TEST_TRUE(reads_float("3.4028235e38", FLT_MAX, 12));
    _TEST_TRUE(reads_float("0x1p-126", FLT_MIN, 8));
    _TEST_TRUE(reads_float("0x1.000001p0", 1.0F, 12));
    _TEST_TRUE(reads_float("0x1.000003p0", 0x1.000004p0F, 12));
    _TEST_TRUE(reads_float("0.1", 0.1F, 3));
    errno = 0;
    _TEST_TRUE(strtof("3.5e38", NULL) > FLT_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_TRUE(strtof("1e-50", NULL) <= FLT_MIN);
}

int main(void)
{
    Test_StdlibStrtof();
    return _Test_Status();
}
