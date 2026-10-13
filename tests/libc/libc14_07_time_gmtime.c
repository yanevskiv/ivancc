// (Test) Status: 0
// A test file for <time.h> gmtime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

#define NONE   (-1)
#define MONTHS 12
#define WEEK   7
#define MONDAY 1
#define BEFORE 14
#define Y2001  978307200L

// [C99:S7.23.3.3p2] gmtime should break a time down into the fields given.
static bool breaks(time_t when, int year, int mon, int mday, int hour, int min, int sec, int wday, int yday)
{
    return is(gmtime(&when), year, mon, mday, hour, min, sec, wday, yday);
}

// gmtime should break down 2001's every 15th: return the first month it errs.
static int misdated(void)
{
    int mon;
    static const int days[MONTHS] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    for (mon = 0; mon < MONTHS; mon++) {
        if (! breaks(Y2001 + (days[mon] + BEFORE) * DAY, 2001, mon, BEFORE + 1, 0, 0, 0, (MONDAY + days[mon] + BEFORE) % WEEK, days[mon] + BEFORE)) {
            return mon;
        }
    }
    return NONE;
}

// [C99:S7.23.3.3p3] gmtime should break down any time whose year an int holds.
static void Test_TimeGmtime(void)
{
    time_t when = 0;

    _TEST_TRUE(breaks(0, 1970, 0, 1, 0, 0, 0, 4, 0));
    _TEST_INT(gmtime(&when)->tm_isdst, 0);
    _TEST_TRUE(breaks(-1, 1969, 11, 31, 23, 59, 59, 3, 364));
    _TEST_TRUE(breaks(LEAP_DAY, 2000, 1, 29, 0, 0, 0, 2, 59));
    _TEST_TRUE(breaks(4107542400, 2100, 2, 1, 0, 0, 0, 1, 59));
    _TEST_TRUE(breaks(Y2038, 2038, 0, 19, 3, 14, 7, 2, 18));
    _TEST_TRUE(breaks(-2208988800, 1900, 0, 1, 0, 0, 0, 1, 0));
    _TEST_TRUE(breaks(-62135596800, 1, 0, 1, 0, 0, 0, 1, 0));
    _TEST_INT(misdated(), NONE);
    when = LONG_MAX;
    _TEST_TRUE(gmtime(&when) == NULL);
    when = LONG_MIN;
    _TEST_TRUE(gmtime(&when) == NULL);
}

int main(void)
{
    Test_TimeGmtime();
    return _Test_Status();
}
