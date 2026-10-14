// (Test) Status: 0
// A test file for <time.h> asctime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99:7.23.3.1p2] asctime should write "Www Mmm dd hh:mm:ss yyyy\n".
static void Test_TimeAsctime(void)
{
    time_t when = 0;
    struct tm tm = make(1973, 8, 16, 1, 3, 52);

    tm.tm_wday = 0;
    _TEST_STR(asctime(&tm), "Sun Sep 16 01:03:52 1973\n");
    _TEST_STR(asctime(gmtime(&when)), "Thu Jan  1 00:00:00 1970\n");
    when = LEAP_DAY + DAY - 1;
    _TEST_STR(asctime(gmtime(&when)), "Tue Feb 29 23:59:59 2000\n");
}

int main(void)
{
    Test_TimeAsctime();
    return _Test_Status();
}
