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

#ifndef __LIBC_TIME_H__
#define __LIBC_TIME_H__

// The types the functions take and return.
#include <time.h>

// The truth values the helpers answer with.
#include <stdbool.h>

// The units of time.
#define __LIBC_TIME_NSECS_PER_CLOCK 1000L
#define __LIBC_TIME_SECS_PER_MIN    60L
#define __LIBC_TIME_SECS_PER_HOUR   3600L
#define __LIBC_TIME_SECS_PER_DAY    86400L
#define __LIBC_TIME_MINS_PER_HOUR   60L
#define __LIBC_TIME_DAYS_PER_WEEK   7L
#define __LIBC_TIME_HOURS_PER_HALF  12

// The cycles of the Gregorian calendar.
#define __LIBC_TIME_DAYS_PER_YEAR     365L
#define __LIBC_TIME_DAYS_PER_CENTURY  36524L
#define __LIBC_TIME_DAYS_PER_ERA      146097L
#define __LIBC_TIME_YEARS_PER_LEAP    4L
#define __LIBC_TIME_YEARS_PER_CENTURY 100L
#define __LIBC_TIME_YEARS_PER_ERA     400L
#define __LIBC_TIME_MONTHS_PER_YEAR   12

// The year a broken-down year counts from.
#define __LIBC_TIME_YEAR_BASE 1900L

// The days from the era that starts on 0000-03-01 to the epoch.
#define __LIBC_TIME_ERA_TO_EPOCH 719468L

// The weekday of the epoch.
#define __LIBC_TIME_EPOCH_WDAY 4L

// January and March as broken-down months.
#define __LIBC_TIME_JANUARY 0
#define __LIBC_TIME_MARCH   2

// The first day of a month.
#define __LIBC_TIME_FIRST_MDAY 1

// The weekday ISO 8601 starts a week on.
#define __LIBC_TIME_ISO_START_WDAY 1L

// The weekday every first week of an ISO 8601 year holds.
#define __LIBC_TIME_ISO_WEEK1_WDAY 4L

// A multiple of a week that keeps a weekday's remainder positive.
#define __LIBC_TIME_ISO_SHIFT 378L

// The base a number is written in.
#define __LIBC_TIME_NUMBER_BASE 10

// The size of a long's digits.
#define __LIBC_TIME_NUMBER_SIZE 24

// The widths asctime and strftime write numbers in.
#define __LIBC_TIME_WIDTH_YEAR   0
#define __LIBC_TIME_WIDTH_WDAY   1
#define __LIBC_TIME_WIDTH_FIELD  2
#define __LIBC_TIME_WIDTH_YDAY   3
#define __LIBC_TIME_WIDTH_MDAY   3
#define __LIBC_TIME_WIDTH_OFFSET 4

// The weight of the hours in a zone's offset as hhmm.
#define __LIBC_TIME_OFFSET_HOUR 100

// The characters of a name asctime and strftime abbreviate.
#define __LIBC_TIME_ABBREV_LEN 3

// The names asctime and strftime write for a weekday or month out of range.
#define __LIBC_TIME_UNKNOWN_ABBREV "???"
#define __LIBC_TIME_UNKNOWN_NAME   "?"

// The size of asctime's widest text.
#define __LIBC_TIME_ASCTIME_SIZE 72

// The zones gmtime and localtime break a time down in.
#define __LIBC_TIME_ZONE_GMT "GMT"
#define __LIBC_TIME_ZONE_UTC "UTC"

// The conversions strftime knows alone, after E and after O.
#define __LIBC_TIME_CONVERSIONS   "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%"
#define __LIBC_TIME_CONVERSIONS_E "cCnprRtTuxXyYzZ%"
#define __LIBC_TIME_CONVERSIONS_O "bBCdeghGHIjmMnprRStTuUVwWyzZ%"

// Text written into a buffer.
struct __libc_time_text {
    char   *tt_str;
    size_t  tt_size;
    size_t  tt_len;
};

// (S7.23.2) Time manipulation functions
clock_t __libc_time_clock(void);
double __libc_time_difftime(time_t time1, time_t time0);
time_t __libc_time_mktime(struct tm *timeptr);
time_t __libc_time_time(time_t *timer);

// (S7.23.3) Time conversion functions
char *__libc_time_asctime(const struct tm *timeptr);
char *__libc_time_ctime(const time_t *timer);
struct tm *__libc_time_gmtime(const time_t *timer);
struct tm *__libc_time_localtime(const time_t *timer);
size_t __libc_time_strftime(char *restrict str, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);

// Calendar
long __libc_time_floor_div(long value, long divisor);
long __libc_time_floor_mod(long value, long divisor);
long __libc_time_year_days(long year);
long __libc_time_month_days(long year, int mon);
struct tm *__libc_time_break_down(time_t value, struct tm *tm, const char *zone);

// Text
void __libc_time_put_char(struct __libc_time_text *text, char ch);
void __libc_time_put_string(struct __libc_time_text *text, const char *str, size_t len);
void __libc_time_put_number(struct __libc_time_text *text, long value, int width, char pad);
void __libc_time_put_two_digits(struct __libc_time_text *text, int value);
const char *__libc_time_wday_name(int wday);
const char *__libc_time_mon_name(int mon);
void __libc_time_put_name(struct __libc_time_text *text, const char *name, size_t len, const char *unknown);

// strftime
long __libc_time_iso_days(long yday, long wday);
long __libc_time_iso_year(const struct tm *tm, long *days);
bool __libc_time_is_conversion(char mod, char conv);
const char *__libc_time_composite(char conv);
void __libc_time_put_conversion(struct __libc_time_text *text, char conv, const struct tm *tm);
void __libc_time_put_format(struct __libc_time_text *text, const char *format, const struct tm *tm);

#endif // __LIBC_TIME_H__
