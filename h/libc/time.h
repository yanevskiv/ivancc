/*
 * C header file for date and time.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _TIME_H
#define _TIME_H

// The units of time.
#define _TIME_NSECS_PER_CLOCK 1000L
#define _TIME_SECS_PER_MIN    60L
#define _TIME_SECS_PER_HOUR   3600L
#define _TIME_SECS_PER_DAY    86400L
#define _TIME_MINS_PER_HOUR   60L
#define _TIME_DAYS_PER_WEEK   7L
#define _TIME_HOURS_PER_HALF  12

// The cycles of the Gregorian calendar.
#define _TIME_DAYS_PER_YEAR     365L
#define _TIME_DAYS_PER_CENTURY  36524L
#define _TIME_DAYS_PER_ERA      146097L
#define _TIME_YEARS_PER_LEAP    4L
#define _TIME_YEARS_PER_CENTURY 100L
#define _TIME_YEARS_PER_ERA     400L
#define _TIME_MONTHS_PER_YEAR   12

// The year a broken-down year counts from.
#define _TIME_YEAR_BASE 1900L

// The days from the era that starts on 0000-03-01 to the epoch.
#define _TIME_ERA_TO_EPOCH 719468L

// The weekday of the epoch.
#define _TIME_EPOCH_WDAY 4L

// January and March as broken-down months.
#define _TIME_JANUARY 0
#define _TIME_MARCH   2

// The first day of a month.
#define _TIME_FIRST_MDAY 1

// The weekday ISO 8601 starts a week on.
#define _TIME_ISO_START_WDAY 1L

// The weekday every first week of an ISO 8601 year holds.
#define _TIME_ISO_WEEK1_WDAY 4L

// A multiple of a week that keeps a weekday's remainder positive.
#define _TIME_ISO_SHIFT 378L

// The base a number is written in.
#define _TIME_NUMBER_BASE 10

// The size of a long's digits.
#define _TIME_NUMBER_SIZE 24

// The widths asctime and strftime write numbers in.
#define _TIME_WIDTH_YEAR   0
#define _TIME_WIDTH_WDAY   1
#define _TIME_WIDTH_FIELD  2
#define _TIME_WIDTH_YDAY   3
#define _TIME_WIDTH_MDAY   3
#define _TIME_WIDTH_OFFSET 4

// The weight of the hours in a zone's offset as hhmm.
#define _TIME_OFFSET_HOUR 100

// The characters of a name asctime and strftime abbreviate.
#define _TIME_ABBREV_LEN 3

// The names asctime and strftime write for a weekday or month out of range.
#define _TIME_UNKNOWN_ABBREV "???"
#define _TIME_UNKNOWN_NAME   "?"

// The size of asctime's widest text.
#define _TIME_ASCTIME_SIZE 72

// The zones gmtime and localtime break a time down in.
#define _TIME_ZONE_GMT "GMT"
#define _TIME_ZONE_UTC "UTC"

// The conversions strftime knows alone, after E and after O.
#define _TIME_CONVERSIONS   "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%"
#define _TIME_CONVERSIONS_E "cCnprRtTuxXyYzZ%"
#define _TIME_CONVERSIONS_O "bBCdeghGHIjmMnprRStTuUVwWyzZ%"

// (S7.23.1) Components of time
#define NULL           ((void *) 0)
#define CLOCKS_PER_SEC ((clock_t) 1000000)

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

typedef long clock_t;
typedef long time_t;

// Text written into a buffer.
struct _Time_Text {
    char   *tt_str;
    size_t tt_size;
    size_t tt_len;
};

// (S7.23.1) Components of time
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long _Time_TmGmtoff;
    const char *_Time_TmZone;
};

// The days before each month of a year that starts in March.
extern const long _Time_MarchDays[_TIME_MONTHS_PER_YEAR];

// The names of the weekdays.
extern const char *const _Time_WdayNames[_TIME_DAYS_PER_WEEK];

// The names of the months.
extern const char *const _Time_MonNames[_TIME_MONTHS_PER_YEAR];

// The broken-down time gmtime and localtime return.
extern struct tm _Time_Tm;

// The text asctime and ctime return.
extern char _Time_AsctimeText[_TIME_ASCTIME_SIZE];

// Calendar
long _Time_FloorDiv(long value, long divisor);
long _Time_FloorMod(long value, long divisor);
long _Time_YearDays(long year);
long _Time_MonthDays(long year, int mon);
struct tm *_Time_BreakDown(time_t value, struct tm *tm, const char *zone);

// Text
void _Time_PutChar(struct _Time_Text *text, char ch);
void _Time_PutString(struct _Time_Text *text, const char *str, size_t len);
void _Time_PutNumber(struct _Time_Text *text, long value, int width, char pad);
void _Time_PutTwoDigits(struct _Time_Text *text, int value);
const char *_Time_WdayName(int wday);
const char *_Time_MonName(int mon);
void _Time_PutName(struct _Time_Text *text, const char *name, size_t len, const char *unknown);

// strftime
long _Time_IsoDays(long yday, long wday);
long _Time_IsoYear(const struct tm *tm, long *days);
_Bool _Time_IsConversion(char mod, char conv);
const char *_Time_Composite(char conv);
void _Time_PutConversion(struct _Time_Text *text, char conv, const struct tm *tm);
void _Time_PutFormat(struct _Time_Text *text, const char *format, const struct tm *tm);

// (S7.23.2) Time manipulation functions
clock_t clock(void);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);

// (S7.23.3) Time conversion functions
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timer);
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);
size_t strftime(char *restrict str, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);

#endif // _TIME_H
