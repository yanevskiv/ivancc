// (Test) Status: 0
// A test file for <limits.h> long and long long limits.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// [C99:5.2.4.2.1p1] long and long long should have 64 bits.
static void Test_LimitsLong(void)
{
    _TEST_INT(LONG_MIN, -9223372036854775807L - 1);
    _TEST_INT(LONG_MAX, 9223372036854775807L);
    _TEST_UINT(ULONG_MAX, 18446744073709551615UL);
    _TEST_INT(LLONG_MIN, -9223372036854775807LL - 1);
    _TEST_INT(LLONG_MAX, 9223372036854775807LL);
    _TEST_UINT(ULLONG_MAX, 18446744073709551615ULL);
}

int main(void)
{
    Test_LimitsLong();
    return _Test_Status();
}
