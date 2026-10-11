// (Test) Status: 0
// <time.h> defines CLOCKS_PER_SEC, size_t, clock_t, time_t and struct tm (S7.23.1), clock, which never runs back, difftime, mktime, which brings each field into its range and sets the days of the week and year, and time (S7.23.2),
// asctime, ctime, which is asctime of localtime, gmtime, which breaks a time down in UTC, before 1970 and across leap years too, and gives none for a year past an int, and localtime, which mktime undoes (S7.23.3.1-4),
// and strftime, with each conversion of the "C" locale, the ISO 8601 week at each turn of a year, the E and O modifiers, and nothing for a text its buffer cannot hold (S7.23.3.5), the same in any time zone,
// each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

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

#define NONE       (-1)
#define SPINS      100000
#define TEXT_SIZE  256
#define YEAR_BASE  1900
#define YEAR_2020  120
#define UNSET_WDAY 99
#define UNSET_YDAY 999
#define MONTHS     12
#define WEEK       7
#define MONDAY     1
#define BEFORE     14
#define DAY        86400L
#define Y2001      978307200L
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

// (S7.23.2.3p2) mktime should bring a broken-down time's fields to those given.
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

// (S7.23.3.3p2) gmtime should break a time down into the fields given.
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

// (S7.23.3.5p3) strftime should format a time in UTC as the text given.
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

// (S7.23.2.1p3) The processor time should be known, and never run back.
static void Test_TimeClock(void)
{
    clock_t start = clock();
    volatile long spin = 0;

    _TEST_TRUE(start != (clock_t) -1);
    _TEST_TRUE(CLOCKS_PER_SEC > 0);
    while (spin < SPINS) {
        spin++;
    }
    _TEST_TRUE(clock() >= start);
}

// (S7.23.2.2p3) difftime should give the seconds between two times, far apart.
static void Test_TimeDifference(void)
{
    _TEST_FLOAT(difftime(10, 4), 6.0);
    _TEST_FLOAT(difftime(4, 10), -6.0);
    _TEST_FLOAT(difftime(7, 7), 0.0);
    _TEST_FLOAT(difftime(LONG_MAX, LONG_MIN), 18446744073709551615.0);
    _TEST_FLOAT(difftime(LONG_MIN, LONG_MAX), -18446744073709551615.0);
}

// (S7.23.2.3p2) mktime should bring fields into range and set the day counts.
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

// (S7.23.2.4p3) time should give the calendar time, store it, and not go back.
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

// (S7.23.3.1p2) asctime should write "Www Mmm dd hh:mm:ss yyyy\n".
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

// (S7.23.3.2p2) ctime should write a time as asctime writes its local time.
static void Test_TimeCtime(void)
{
    time_t now = time(NULL);
    time_t when = 0;
    char buf[TEXT_SIZE];

    strcpy(buf, ctime(&when));
    _TEST_STR(asctime(localtime(&when)), buf);
    strcpy(buf, ctime(&now));
    _TEST_STR(asctime(localtime(&now)), buf);
}

// (S7.23.3.3p3) gmtime should break down any time whose year an int holds.
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

// (S7.23.3.4p2) localtime should break a time down into fields within range.
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

// (S7.23.3.5p7) strftime should write each conversion as the "C" locale does.
static void Test_TimeConversions(void)
{
    _TEST_TRUE(formats(LAST_1999, "%a|%A|%b|%B|%c|%C|%d|%D|%e|%F|%g|%G|%h|%H|%I|%j|%m", "Fri|Friday|Dec|December|Fri Dec 31 23:59:58 1999|19|31|12/31/99|31|1999-12-31|99|1999|Dec|23|11|365|12"));
    _TEST_TRUE(formats(LAST_1999, "%M|%n|%p|%r|%R|%S|%t|%T|%u|%U|%V|%w|%W|%x|%X|%y|%Y|%%", "59|\n|PM|11:59:58 PM|23:59|58|\t|23:59:58|5|52|52|5|52|12/31/99|23:59:58|99|1999|%"));
    _TEST_TRUE(formats(LAST_1999, "%Ec|%EC|%Ex|%EX|%Ey|%EY", "Fri Dec 31 23:59:58 1999|19|12/31/99|23:59:58|99|1999"));
    _TEST_TRUE(formats(LAST_1999, "%Od|%Oe|%OH|%OI|%Om|%OM|%OS|%Ou|%OU|%OV|%Ow|%OW|%Oy", "31|31|23|11|12|59|58|5|52|52|5|52|99"));
}

// (S7.23.3.5p5) A year's ends should fall in an ISO 8601 week of either year.
static void Test_TimeWeeks(void)
{
    _TEST_TRUE(formats(1104537600 + 309, "%G-W%V-%u %g %U %W %e %I %p %j", "2004-W53-6 04 00 00  1 12 AM 001"));
    _TEST_TRUE(formats(1230508800 + 43200, "%G-W%V-%u %g %U %W %I %p", "2009-W01-1 09 52 52 12 PM"));
    _TEST_TRUE(formats(1262476800 + 46800, "%G-W%V-%u %g %U %W %w %I %p", "2009-W53-7 09 01 00 0 01 PM"));
    _TEST_TRUE(formats(1104451200, "%G-W%V-%u %U %W %j", "2004-W53-5 52 52 366"));
    _TEST_TRUE(formats(1136073600, "%G-W%V-%u %U %W", "2005-W52-7 01 00"));
    _TEST_TRUE(formats(1167609600, "%G-W%V-%u %U %W", "2007-W01-1 00 01"));
}

// (S7.23.3.5p8) strftime should count what it wrote, nothing if too long.
static void Test_TimeSizes(void)
{
    time_t when = LAST_1999;
    struct tm tm = *gmtime(&when);
    char buf[TEXT_SIZE];

    _TEST_UINT(strftime(buf, 5, "%Y", &tm), 4);
    _TEST_STR(buf, "1999");
    _TEST_UINT(strftime(buf, 4, "%Y", &tm), 0);
    _TEST_UINT(strftime(buf, 1, "", &tm), 0);
    _TEST_INT(buf[0], '\0');
    _TEST_UINT(strftime(buf, sizeof(buf), "a%%b%nc", &tm), 5);
    _TEST_STR(buf, "a%b\nc");
}

// (S7.1.4p1) A pointer and a call in parentheses should reach each function.
static void Test_TimeCalls(void)
{
    time_t when = 0;
    struct tm tm = make(1973, 8, 16, 1, 3, 52);
    char buf[TEXT_SIZE];
    char *(*text)(const struct tm *) = asctime;
    size_t (*format)(char *restrict, size_t, const char *restrict, const struct tm *restrict) = strftime;

    tm.tm_wday = 0;
    _TEST_STR(text(&tm), "Sun Sep 16 01:03:52 1973\n");
    _TEST_UINT(format(buf, sizeof(buf), "%T", &tm), 8);
    _TEST_STR(buf, "01:03:52");

    _TEST_TRUE((clock)() != (clock_t) -1);
    _TEST_FLOAT((difftime)(2, 1), 1.0);
    _TEST_TRUE((mktime)(&tm) != (time_t) -1);
    _TEST_TRUE((time)(NULL) != (time_t) -1);
    _TEST_STR((asctime)(&tm), "Sun Sep 16 01:03:52 1973\n");
    strcpy(buf, (ctime)(&when));
    _TEST_STR(asctime(localtime(&when)), buf);
    _TEST_INT((gmtime)(&when)->tm_year, 70);
    _TEST_TRUE((localtime)(&when) != NULL);
    _TEST_UINT((strftime)(buf, sizeof(buf), "%Y", &tm), 4);
    _TEST_STR(buf, "1973");
}

int main(void)
{
    Test_TimeClock();
    Test_TimeDifference();
    Test_TimeMktime();
    Test_TimeNow();
    Test_TimeAsctime();
    Test_TimeCtime();
    Test_TimeGmtime();
    Test_TimeLocaltime();
    Test_TimeConversions();
    Test_TimeWeeks();
    Test_TimeSizes();
    Test_TimeCalls();
    return _Test_Status();
}
