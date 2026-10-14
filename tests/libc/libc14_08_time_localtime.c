// (Test) Status: 0
// A test file for <time.h> localtime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// localtime should break a time down into fields within range.
// [C99:7.23.3.4p2]: localtime
static void Test_TimeLocaltime(void)
{
    time_t now = time(NULL);
    const struct tm *tm = localtime(&now);

    if (_TEST_TRUE(tm != NULL)) {
        _TEST_TRUE(tm->tm_sec >= 0 && tm->tm_sec <= 60);
        _TEST_TRUE(tm->tm_min >= 0 && tm->tm_min <= 59);
        _TEST_TRUE(tm->tm_hour >= 0 && tm->tm_hour <= 23);
        _TEST_TRUE(tm->tm_mday >= 1 && tm->tm_mday <= 31);
        _TEST_TRUE(tm->tm_mon >= 0 && tm->tm_mon <= 11);
        _TEST_TRUE(tm->tm_year >= YEAR_2020);
        _TEST_TRUE(tm->tm_wday >= 0 && tm->tm_wday <= 6);
        _TEST_TRUE(tm->tm_yday >= 0 && tm->tm_yday <= 365);
    }
}

int main(void)
{
    Test_TimeLocaltime();
    return _Test_Status();
}
