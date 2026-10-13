// (Test) Status: 0
// A test file for <time.h> time.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99 S7.23.2.4p3] time should give the calendar time, store it, and not go back.
static void Test_TimeNow(void)
{
    time_t now = 0;
    time_t when;

    _TEST_TRUE(time(&now) != (time_t) -1);
    _TEST_TRUE(now != 0);
    _TEST_TRUE(time(NULL) >= now);
    when = time(&now);
    _TEST_INT(when, now);
    _TEST_TRUE(gmtime(&now)->tm_year >= YEAR_2020);
}

int main(void)
{
    Test_TimeNow();
    return _Test_Status();
}
