// (Test) Status: 0
// A test file for <time.h> difftime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99 S7.23.2.2p3] difftime should give the seconds between two times, far apart.
static void Test_TimeDifference(void)
{
    _TEST_FLOAT(difftime(10, 4), 6.0);
    _TEST_FLOAT(difftime(4, 10), -6.0);
    _TEST_FLOAT(difftime(7, 7), 0.0);
    _TEST_FLOAT(difftime(LONG_MAX, LONG_MIN), 18446744073709551615.0);
    _TEST_FLOAT(difftime(LONG_MIN, LONG_MAX), -18446744073709551615.0);
}

int main(void)
{
    Test_TimeDifference();
    return _Test_Status();
}
