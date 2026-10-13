// (Test) Status: 0
// A test file for <time.h> mktime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99 S7.23.2.3p2] mktime should bring a broken-down time's fields to those given.
static bool normalizes(struct tm tm, int year, int mon, int mday, int hour, int min, int sec, int wday, int yday)
{
    int lost = 0;

    lost += ! _TEST_TRUE(mktime(&tm) != (time_t) -1);
    lost += ! is(&tm, year, mon, mday, hour, min, sec, wday, yday);
    return lost == 0;
}

// Return the seconds mktime puts between two broken-down times.
static double apart(struct tm later, struct tm earlier)
{
    return difftime(mktime(&later), mktime(&earlier));
}

// Return the time mktime makes of what localtime broke a time down into.
static time_t back(time_t when)
{
    struct tm tm = *localtime(&when);

    return mktime(&tm);
}

// [C99 S7.23.2.3p2] mktime should bring fields into range and set the day counts.
static void Test_TimeMktime(void)
{
    time_t now = time(NULL);

    _TEST_TRUE(normalizes(make(2000, 0, 32, 12, 0, 0), 2000, 1, 1, 12, 0, 0, 2, 31));
    _TEST_TRUE(normalizes(make(1999, 13, 29, 12, 0, 0), 2000, 1, 29, 12, 0, 0, 2, 59));
    _TEST_TRUE(normalizes(make(2100, 1, 29, 12, 0, 0), 2100, 2, 1, 12, 0, 0, 1, 59));
    _TEST_TRUE(normalizes(make(2001, 2, 0, 12, 0, 0), 2001, 1, 28, 12, 0, 0, 3, 58));
    _TEST_TRUE(normalizes(make(2001, -1, 31, 12, 59, 61), 2000, 11, 31, 13, 0, 1, 0, 365));
    _TEST_TRUE(normalizes(make(2001, 0, 1, 0, 0, -1), 2000, 11, 31, 23, 59, 59, 0, 365));

    _TEST_FLOAT(apart(make(2001, 0, 11, 12, 0, 0), make(2001, 0, 10, 12, 0, 0)), 86400.0);
    _TEST_FLOAT(apart(make(2001, 0, 10, 12, 0, 61), make(2001, 0, 10, 12, 0, 0)), 61.0);
    _TEST_FLOAT(apart(make(2001, 2, 1, 0, 0, 0), make(2001, 1, 1, 0, 0, 0)), 2419200.0);
    _TEST_FLOAT(apart(make(2000, 2, 1, 0, 0, 0), make(2000, 1, 1, 0, 0, 0)), 2505600.0);

    _TEST_INT(back(0), 0);
    _TEST_INT(back(LEAP_DAY), LEAP_DAY);
    _TEST_INT(back(Y2038), Y2038);
    _TEST_INT(back(now), now);
}

int main(void)
{
    Test_TimeMktime();
    return _Test_Status();
}
