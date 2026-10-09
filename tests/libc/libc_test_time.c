// (Test) Status: 0
// <time.h> defines the clock, the calendar time and its broken-down and textual forms, the same in any time zone (S7.23).

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

static struct tm make(int year, int mon, int mday, int hour, int min, int sec)
{
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - 1900;
    tm.tm_mon = mon;
    tm.tm_mday = mday;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_wday = 99;
    tm.tm_yday = 999;
    tm.tm_isdst = -1;
    return tm;
}

static bool is(const struct tm *tm, int year, int mon, int mday, int hour, int min, int sec, int wday, int yday)
{
    return tm != NULL and tm->tm_year == year - 1900 and tm->tm_mon == mon and tm->tm_mday == mday and tm->tm_hour == hour and tm->tm_min == min and tm->tm_sec == sec and tm->tm_wday == wday and tm->tm_yday == yday;
}

static struct tm utc(time_t value)
{
    return *gmtime(&value);
}

static bool formats(const struct tm *tm, const char *format, const char *want)
{
    char buf[256];
    size_t len = strftime(buf, sizeof(buf), format, tm);

    return len == strlen(want) and strcmp(buf, want) == 0;
}

static bool round_trips(time_t value)
{
    struct tm tm = *localtime(&value);

    return mktime(&tm) == value;
}

int main(void)
{
    char *(*text)(const struct tm *) = asctime;
    size_t (*format)(char *restrict, size_t, const char *restrict, const struct tm *restrict) = strftime;
    static const int days[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    volatile long spin = 0;
    char buf[64];
    char other[64];
    struct tm tm;
    clock_t start;
    time_t now;
    time_t when;

    start = clock();
    if (start == (clock_t) -1 or CLOCKS_PER_SEC <= 0) return 1;
    while (spin < 100000) spin++;
    if (clock() < start) return 1;

    if (difftime(10, 4) != 6.0 or difftime(4, 10) != -6.0 or difftime(7, 7) != 0.0) return 2;
    if (difftime(LONG_MAX, LONG_MIN) != 18446744073709551615.0 or difftime(LONG_MIN, LONG_MAX) != -18446744073709551615.0) return 2;

    tm = make(2000, 0, 32, 12, 0, 0);
    if (mktime(&tm) == (time_t) -1 or not is(&tm, 2000, 1, 1, 12, 0, 0, 2, 31)) return 3;
    tm = make(1999, 13, 29, 12, 0, 0);
    if (mktime(&tm) == (time_t) -1 or not is(&tm, 2000, 1, 29, 12, 0, 0, 2, 59)) return 3;
    tm = make(2100, 1, 29, 12, 0, 0);
    if (mktime(&tm) == (time_t) -1 or not is(&tm, 2100, 2, 1, 12, 0, 0, 1, 59)) return 3;
    tm = make(2001, 2, 0, 12, 0, 0);
    if (mktime(&tm) == (time_t) -1 or not is(&tm, 2001, 1, 28, 12, 0, 0, 3, 58)) return 3;
    tm = make(2001, -1, 31, 12, 59, 61);
    if (mktime(&tm) == (time_t) -1 or not is(&tm, 2000, 11, 31, 13, 0, 1, 0, 365)) return 3;

    tm = make(2001, 0, 11, 12, 0, 0);
    when = mktime(&tm);
    tm = make(2001, 0, 10, 12, 0, 0);
    if (difftime(when, mktime(&tm)) != 86400.0) return 4;
    tm = make(2001, 0, 10, 12, 0, 61);
    when = mktime(&tm);
    tm = make(2001, 0, 10, 12, 0, 0);
    if (difftime(when, mktime(&tm)) != 61.0) return 4;
    if (not round_trips(0) or not round_trips(951782400) or not round_trips(2147483647) or not round_trips(time(NULL))) return 4;

    now = 0;
    if (time(&now) == (time_t) -1 or time(NULL) < now) return 5;
    when = time(&now);
    if (when != now or gmtime(&now)->tm_year < 120) return 5;

    tm = make(1973, 8, 16, 1, 3, 52);
    tm.tm_wday = 0;
    if (strcmp(asctime(&tm), "Sun Sep 16 01:03:52 1973\n") != 0) return 6;
    tm = utc(0);
    if (strcmp(asctime(&tm), "Thu Jan  1 00:00:00 1970\n") != 0) return 6;
    tm = utc(951782400 + 86399);
    if (strcmp(asctime(&tm), "Tue Feb 29 23:59:59 2000\n") != 0) return 6;

    when = 0;
    strcpy(buf, ctime(&when));
    strcpy(other, asctime(localtime(&when)));
    if (strcmp(buf, other) != 0) return 7;
    strcpy(buf, ctime(&now));
    strcpy(other, asctime(localtime(&now)));
    if (strcmp(buf, other) != 0) return 7;

    when = 0;
    if (not is(gmtime(&when), 1970, 0, 1, 0, 0, 0, 4, 0) or gmtime(&when)->tm_isdst != 0) return 8;
    when = -1;
    if (not is(gmtime(&when), 1969, 11, 31, 23, 59, 59, 3, 364)) return 8;
    when = 951782400;
    if (not is(gmtime(&when), 2000, 1, 29, 0, 0, 0, 2, 59)) return 8;
    when = 4107542400;
    if (not is(gmtime(&when), 2100, 2, 1, 0, 0, 0, 1, 59)) return 8;
    when = 2147483647;
    if (not is(gmtime(&when), 2038, 0, 19, 3, 14, 7, 2, 18)) return 8;
    when = -2208988800;
    if (not is(gmtime(&when), 1900, 0, 1, 0, 0, 0, 1, 0)) return 8;
    when = -62135596800;
    if (not is(gmtime(&when), 1, 0, 1, 0, 0, 0, 1, 0)) return 8;
    when = LONG_MAX;
    if (gmtime(&when) != NULL) return 8;
    for (int mon = 0; mon < 12; mon++) {
        when = (978307200 / 86400 + days[mon] + 14) * 86400L;
        if (not is(gmtime(&when), 2001, mon, 15, 0, 0, 0, (int) ((days[mon] + 15) % 7), days[mon] + 14)) return 8;
    }

    if (localtime(&now) == NULL or localtime(&now)->tm_sec > 60 or localtime(&now)->tm_mon > 11) return 9;

    tm = utc(946684798);
    if (not formats(&tm, "%a|%A|%b|%B|%c|%C|%d|%D|%e|%F|%g|%G|%h|%H|%I|%j|%m", "Fri|Friday|Dec|December|Fri Dec 31 23:59:58 1999|19|31|12/31/99|31|1999-12-31|99|1999|Dec|23|11|365|12")) return 10;
    if (not formats(&tm, "%M|%n|%p|%r|%R|%S|%t|%T|%u|%U|%V|%w|%W|%x|%X|%y|%Y|%%", "59|\n|PM|11:59:58 PM|23:59|58|\t|23:59:58|5|52|52|5|52|12/31/99|23:59:58|99|1999|%")) return 10;

    tm = utc(1104537600 + 309);
    if (not formats(&tm, "%G-W%V-%u %g %U %W %e %I %p %j", "2004-W53-6 04 00 00  1 12 AM 001")) return 11;
    tm = utc(1230508800 + 43200);
    if (not formats(&tm, "%G-W%V-%u %g %U %W %I %p", "2009-W01-1 09 52 52 12 PM")) return 11;
    tm = utc(1262476800 + 46800);
    if (not formats(&tm, "%G-W%V-%u %g %U %W %w %I %p", "2009-W53-7 09 01 00 0 01 PM")) return 11;
    tm = utc(1104451200);
    if (not formats(&tm, "%G-W%V-%u %U %W %j", "2004-W53-5 52 52 366")) return 11;
    tm = utc(1136073600);
    if (not formats(&tm, "%G-W%V-%u %U %W", "2005-W52-7 01 00")) return 11;
    tm = utc(1167609600);
    if (not formats(&tm, "%G-W%V-%u %U %W", "2007-W01-1 00 01")) return 11;

    tm = utc(946684798);
    if (not formats(&tm, "%Ec|%EC|%Ex|%EX|%Ey|%EY", "Fri Dec 31 23:59:58 1999|19|12/31/99|23:59:58|99|1999")) return 12;
    if (not formats(&tm, "%Od|%Oe|%OH|%OI|%Om|%OM|%OS|%Ou|%OU|%OV|%Ow|%OW|%Oy", "31|31|23|11|12|59|58|5|52|52|5|52|99")) return 12;

    if (strftime(buf, 5, "%Y", &tm) != 4 or strcmp(buf, "1999") != 0) return 13;
    if (strftime(buf, 4, "%Y", &tm) != 0) return 13;
    if (strftime(buf, 1, "", &tm) != 0 or buf[0] != '\0') return 13;
    if (strftime(buf, sizeof(buf), "a%%b%nc", &tm) != 5 or strcmp(buf, "a%b\nc") != 0) return 13;

    tm = make(1973, 8, 16, 1, 3, 52);
    tm.tm_wday = 0;
    if (strcmp(text(&tm), "Sun Sep 16 01:03:52 1973\n") != 0 or format(buf, sizeof(buf), "%T", &tm) != 8) return 14;
    if (strcmp((asctime)(&tm), "Sun Sep 16 01:03:52 1973\n") != 0 or (difftime)(2, 1) != 1.0 or (gmtime)(&now) == NULL) return 14;
    return 0;
}
