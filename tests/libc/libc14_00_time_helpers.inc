// Includable helper file for <time.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>

// Defines.
#define TEXT_SIZE  256
#define YEAR_BASE  1900
#define YEAR_2020  120
#define UNSET_WDAY 99
#define UNSET_YDAY 999
#define DAY        86400L
#define LEAP_DAY   951782400L
#define Y2038      2147483647L
#define LAST_1999  946684798L

// Return a broken-down time of the fields given, its two day counts unset.
static struct tm make(int year, int mon, int mday, int hour, int min, int sec)
{
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - YEAR_BASE;
    tm.tm_mon = mon;
    tm.tm_mday = mday;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_wday = UNSET_WDAY;
    tm.tm_yday = UNSET_YDAY;
    tm.tm_isdst = -1;
    return tm;
}

// A broken-down time should have the fields given.
static bool is(const struct tm *tm, int year, int mon, int mday, int hour, int min, int sec, int wday, int yday)
{
    int lost = 0;

    if (! _TEST_TRUE(tm != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(tm->tm_year, year - YEAR_BASE);
    lost += ! _TEST_INT(tm->tm_mon, mon);
    lost += ! _TEST_INT(tm->tm_mday, mday);
    lost += ! _TEST_INT(tm->tm_hour, hour);
    lost += ! _TEST_INT(tm->tm_min, min);
    lost += ! _TEST_INT(tm->tm_sec, sec);
    lost += ! _TEST_INT(tm->tm_wday, wday);
    lost += ! _TEST_INT(tm->tm_yday, yday);
    return lost == 0;
}

// [C99 S7.23.3.5p3] strftime should format a time in UTC as the text given.
static bool formats(time_t when, const char *format, const char *want)
{
    int lost = 0;
    struct tm tm = *gmtime(&when);
    char buf[TEXT_SIZE];
    size_t len = strftime(buf, sizeof(buf), format, &tm);

    if (len == 0) {
        buf[0] = '\0';
    }
    lost += ! _TEST_STR(buf, want);
    lost += ! _TEST_UINT(len, strlen(want));
    return lost == 0;
}
