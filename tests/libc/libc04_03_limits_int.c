// (Test) Status: 0
// A test file for <limits.h> short and int limits.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// [C99 S5.2.4.2.1p1] short should have 16 bits and int 32.
static void Test_LimitsInt(void)
{
    _TEST_INT(SHRT_MIN, -32768);
    _TEST_INT(SHRT_MAX, 32767);
    _TEST_INT(USHRT_MAX, 65535);
    _TEST_INT(INT_MIN, -2147483647 - 1);
    _TEST_INT(INT_MAX, 2147483647);
    _TEST_INT(UINT_MAX, 4294967295L);
}

int main(void)
{
    Test_LimitsInt();
    return _Test_Status();
}
