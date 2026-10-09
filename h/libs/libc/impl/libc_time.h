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

#ifndef _LIBC_IMPL_TIME_H
#define _LIBC_IMPL_TIME_H

// The sizes and null pointer the functions take.
#include <libc/impl/libc_stddef.h>

// Components of time
#define _LIBC_IMPL_TIME_CLOCKS_PER_SEC ((_Libc_Impl_Time_clock_t) 1000000)

typedef long _Libc_Impl_Time_clock_t;
typedef long _Libc_Impl_Time_time_t;

// The units of time.
#define _LIBC_IMPL_TIME_NSECS_PER_CLOCK 1000L
#define _LIBC_IMPL_TIME_SECS_PER_MIN    60L
#define _LIBC_IMPL_TIME_SECS_PER_HOUR   3600L
#define _LIBC_IMPL_TIME_SECS_PER_DAY    86400L
#define _LIBC_IMPL_TIME_MINS_PER_HOUR   60L
#define _LIBC_IMPL_TIME_DAYS_PER_WEEK   7L
#define _LIBC_IMPL_TIME_HOURS_PER_HALF  12

// The cycles of the Gregorian calendar.
#define _LIBC_IMPL_TIME_DAYS_PER_YEAR     365L
#define _LIBC_IMPL_TIME_DAYS_PER_CENTURY  36524L
#define _LIBC_IMPL_TIME_DAYS_PER_ERA      146097L
#define _LIBC_IMPL_TIME_YEARS_PER_LEAP    4L
#define _LIBC_IMPL_TIME_YEARS_PER_CENTURY 100L
#define _LIBC_IMPL_TIME_YEARS_PER_ERA     400L
#define _LIBC_IMPL_TIME_MONTHS_PER_YEAR   12

// The year a broken-down year counts from.
#define _LIBC_IMPL_TIME_YEAR_BASE 1900L

// The days from the era that starts on 0000-03-01 to the epoch.
#define _LIBC_IMPL_TIME_ERA_TO_EPOCH 719468L

// The weekday of the epoch.
#define _LIBC_IMPL_TIME_EPOCH_WDAY 4L

// January and March as broken-down months.
#define _LIBC_IMPL_TIME_JANUARY 0
#define _LIBC_IMPL_TIME_MARCH   2

// The first day of a month.
#define _LIBC_IMPL_TIME_FIRST_MDAY 1

// The weekday ISO 8601 starts a week on.
#define _LIBC_IMPL_TIME_ISO_START_WDAY 1L

// The weekday every first week of an ISO 8601 year holds.
#define _LIBC_IMPL_TIME_ISO_WEEK1_WDAY 4L

// A multiple of a week that keeps a weekday's remainder positive.
#define _LIBC_IMPL_TIME_ISO_SHIFT 378L

// The base a number is written in.
#define _LIBC_IMPL_TIME_NUMBER_BASE 10

// The size of a long's digits.
#define _LIBC_IMPL_TIME_NUMBER_SIZE 24

// The widths asctime and strftime write numbers in.
#define _LIBC_IMPL_TIME_WIDTH_YEAR   0
#define _LIBC_IMPL_TIME_WIDTH_WDAY   1
#define _LIBC_IMPL_TIME_WIDTH_FIELD  2
#define _LIBC_IMPL_TIME_WIDTH_YDAY   3
#define _LIBC_IMPL_TIME_WIDTH_MDAY   3
#define _LIBC_IMPL_TIME_WIDTH_OFFSET 4

// The weight of the hours in a zone's offset as hhmm.
#define _LIBC_IMPL_TIME_OFFSET_HOUR 100

// The characters of a name asctime and strftime abbreviate.
#define _LIBC_IMPL_TIME_ABBREV_LEN 3

// The names asctime and strftime write for a weekday or month out of range.
#define _LIBC_IMPL_TIME_UNKNOWN_ABBREV "???"
#define _LIBC_IMPL_TIME_UNKNOWN_NAME   "?"

// The size of asctime's widest text.
#define _LIBC_IMPL_TIME_ASCTIME_SIZE 72

// The zones gmtime and localtime break a time down in.
#define _LIBC_IMPL_TIME_ZONE_GMT "GMT"
#define _LIBC_IMPL_TIME_ZONE_UTC "UTC"

// The conversions strftime knows alone, after E and after O.
#define _LIBC_IMPL_TIME_CONVERSIONS   "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%"
#define _LIBC_IMPL_TIME_CONVERSIONS_E "cCnprRtTuxXyYzZ%"
#define _LIBC_IMPL_TIME_CONVERSIONS_O "bBCdeghGHIjmMnprRStTuUVwWyzZ%"

// The broken-down time, under C99's tag, which no typedef renames.
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
    long _Libc_tm_gmtoff;
    const char *_Libc_tm_zone;
};

// Text written into a buffer.
struct _Libc_Impl_Time_Text {
    char                      *tt_str;
    _Libc_Impl_Stddef_size_t  tt_size;
    _Libc_Impl_Stddef_size_t  tt_len;
};

// Time manipulation functions
_Libc_Impl_Time_clock_t _Libc_Impl_Time_clock(void);
double _Libc_Impl_Time_difftime(_Libc_Impl_Time_time_t time1, _Libc_Impl_Time_time_t time0);
_Libc_Impl_Time_time_t _Libc_Impl_Time_mktime(struct tm *timeptr);
_Libc_Impl_Time_time_t _Libc_Impl_Time_time(_Libc_Impl_Time_time_t *timer);

// Time conversion functions
char *_Libc_Impl_Time_asctime(const struct tm *timeptr);
char *_Libc_Impl_Time_ctime(const _Libc_Impl_Time_time_t *timer);
struct tm *_Libc_Impl_Time_gmtime(const _Libc_Impl_Time_time_t *timer);
struct tm *_Libc_Impl_Time_localtime(const _Libc_Impl_Time_time_t *timer);
_Libc_Impl_Stddef_size_t _Libc_Impl_Time_strftime(char *restrict str, _Libc_Impl_Stddef_size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);

// Calendar
long _Libc_Impl_Time_FloorDiv(long value, long divisor);
long _Libc_Impl_Time_FloorMod(long value, long divisor);
long _Libc_Impl_Time_YearDays(long year);
long _Libc_Impl_Time_MonthDays(long year, int mon);
struct tm *_Libc_Impl_Time_BreakDown(_Libc_Impl_Time_time_t value, struct tm *tm, const char *zone);

// Text
void _Libc_Impl_Time_PutChar(struct _Libc_Impl_Time_Text *text, char ch);
void _Libc_Impl_Time_PutString(struct _Libc_Impl_Time_Text *text, const char *str, _Libc_Impl_Stddef_size_t len);
void _Libc_Impl_Time_PutNumber(struct _Libc_Impl_Time_Text *text, long value, int width, char pad);
void _Libc_Impl_Time_PutTwoDigits(struct _Libc_Impl_Time_Text *text, int value);
const char *_Libc_Impl_Time_WdayName(int wday);
const char *_Libc_Impl_Time_MonName(int mon);
void _Libc_Impl_Time_PutName(struct _Libc_Impl_Time_Text *text, const char *name, _Libc_Impl_Stddef_size_t len, const char *unknown);

// strftime
long _Libc_Impl_Time_IsoDays(long yday, long wday);
long _Libc_Impl_Time_IsoYear(const struct tm *tm, long *days);
_Bool _Libc_Impl_Time_IsConversion(char mod, char conv);
const char *_Libc_Impl_Time_Composite(char conv);
void _Libc_Impl_Time_PutConversion(struct _Libc_Impl_Time_Text *text, char conv, const struct tm *tm);
void _Libc_Impl_Time_PutFormat(struct _Libc_Impl_Time_Text *text, const char *format, const struct tm *tm);

#endif // _LIBC_IMPL_TIME_H
