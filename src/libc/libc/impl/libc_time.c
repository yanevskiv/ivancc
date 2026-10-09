/*
 * C source file for date and time.
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

// Module header.
#include <libc/impl/libc_time.h>

// The range of a broken-down year.
#include <limits.h>

// The length of a name written whole.
#include <stdint.h>

// The error number of a time out of range.
#include <errno.h>

// The conversions strftime knows.
#include <libc/impl/libc_string.h>

// The clocks clock and time read.
#include <libc/libc_sys.h>

// The days before each month of a year that starts in March.
static const long __libc_impl_time_march_days[__LIBC_IMPL_TIME_MONTHS_PER_YEAR] = { 0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337 };

// The names of the weekdays.
static const char *const __libc_impl_time_wday_names[__LIBC_IMPL_TIME_DAYS_PER_WEEK] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// The names of the months.
static const char *const __libc_impl_time_mon_names[__LIBC_IMPL_TIME_MONTHS_PER_YEAR] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

// The broken-down time gmtime and localtime return.
static struct tm __libc_impl_time_tm;

// The text asctime and ctime return.
static char __libc_impl_time_asctime_text[__LIBC_IMPL_TIME_ASCTIME_SIZE];

// Return the processor time the program has used.
clock_t __libc_impl_time_clock(void)
{
    struct __libc_sys_timespec spec;

    if (__libc_sys_clock_gettime(__LIBC_SYS_CLOCK_PROCESS_CPUTIME_ID, &spec) != 0) {
        return (clock_t) -1;
    }
    return spec.tv_sec * CLOCKS_PER_SEC + spec.tv_nsec / __LIBC_IMPL_TIME_NSECS_PER_CLOCK;
}

// Return the seconds from time0 to time1.
double __libc_impl_time_difftime(time_t time1, time_t time0)
{
    if (time1 >= time0) {
        return (double) ((unsigned long) time1 - (unsigned long) time0);
    }
    return -(double) ((unsigned long) time0 - (unsigned long) time1);
}

// Convert the local time timeptr to a calendar time.
time_t __libc_impl_time_mktime(struct tm *timeptr)
{
    long year = timeptr->tm_year + __LIBC_IMPL_TIME_YEAR_BASE + __libc_impl_time_floor_div(timeptr->tm_mon, __LIBC_IMPL_TIME_MONTHS_PER_YEAR);
    int mon = (int) __libc_impl_time_floor_mod(timeptr->tm_mon, __LIBC_IMPL_TIME_MONTHS_PER_YEAR);
    long days = __libc_impl_time_month_days(year, mon) + timeptr->tm_mday - __LIBC_IMPL_TIME_FIRST_MDAY;
    time_t value = days * __LIBC_IMPL_TIME_SECS_PER_DAY + timeptr->tm_hour * __LIBC_IMPL_TIME_SECS_PER_HOUR + timeptr->tm_min * __LIBC_IMPL_TIME_SECS_PER_MIN + timeptr->tm_sec;
    struct tm result;

    if (__libc_impl_time_break_down(value, &result, __LIBC_IMPL_TIME_ZONE_UTC) == NULL) {
        return (time_t) -1;
    }
    *timeptr = result;
    return value;
}

// Return the current calendar time.
time_t __libc_impl_time_time(time_t *timer)
{
    struct __libc_sys_timespec spec;
    time_t value = (time_t) -1;

    if (__libc_sys_clock_gettime(__LIBC_SYS_CLOCK_REALTIME, &spec) == 0) {
        value = spec.tv_sec;
    }
    if (timer != NULL) {
        *timer = value;
    }
    return value;
}

// Write the broken-down time timeptr as text.
char *__libc_impl_time_asctime(const struct tm *timeptr)
{
    struct __libc_impl_time_text text = {
        .tt_str  = __libc_impl_time_asctime_text,
        .tt_size = __LIBC_IMPL_TIME_ASCTIME_SIZE,
        .tt_len  = 0
    };

    if (timeptr->tm_year > INT_MAX - __LIBC_IMPL_TIME_YEAR_BASE) {
        errno = __LIBC_SYS_EOVERFLOW;
        return NULL;
    }
    __libc_impl_time_put_name(&text, __libc_impl_time_wday_name(timeptr->tm_wday), __LIBC_IMPL_TIME_ABBREV_LEN, __LIBC_IMPL_TIME_UNKNOWN_ABBREV);
    __libc_impl_time_put_char(&text, ' ');
    __libc_impl_time_put_name(&text, __libc_impl_time_mon_name(timeptr->tm_mon), __LIBC_IMPL_TIME_ABBREV_LEN, __LIBC_IMPL_TIME_UNKNOWN_ABBREV);
    __libc_impl_time_put_number(&text, timeptr->tm_mday, __LIBC_IMPL_TIME_WIDTH_MDAY, ' ');
    __libc_impl_time_put_char(&text, ' ');
    __libc_impl_time_put_two_digits(&text, timeptr->tm_hour);
    __libc_impl_time_put_char(&text, ':');
    __libc_impl_time_put_two_digits(&text, timeptr->tm_min);
    __libc_impl_time_put_char(&text, ':');
    __libc_impl_time_put_two_digits(&text, timeptr->tm_sec);
    __libc_impl_time_put_char(&text, ' ');
    __libc_impl_time_put_number(&text, timeptr->tm_year + __LIBC_IMPL_TIME_YEAR_BASE, __LIBC_IMPL_TIME_WIDTH_YEAR, '0');
    __libc_impl_time_put_char(&text, '\n');
    __libc_impl_time_put_char(&text, '\0');
    return __libc_impl_time_asctime_text;
}

// Write the local time of timer as text.
char *__libc_impl_time_ctime(const time_t *timer)
{
    struct tm *tm = __libc_impl_time_localtime(timer);

    if (tm == NULL) {
        return NULL;
    }
    return __libc_impl_time_asctime(tm);
}

// Break the time timer down as a time of UTC.
struct tm *__libc_impl_time_gmtime(const time_t *timer)
{
    return __libc_impl_time_break_down(*timer, &__libc_impl_time_tm, __LIBC_IMPL_TIME_ZONE_GMT);
}

// Break the time timer down as a local time.
struct tm *__libc_impl_time_localtime(const time_t *timer)
{
    return __libc_impl_time_break_down(*timer, &__libc_impl_time_tm, __LIBC_IMPL_TIME_ZONE_UTC);
}

// Write the conversions of timeptr that format names into str.
size_t __libc_impl_time_strftime(char *restrict str, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr)
{
    struct __libc_impl_time_text text = {
        .tt_str  = str,
        .tt_size = maxsize,
        .tt_len  = 0
    };

    __libc_impl_time_put_format(&text, format, timeptr);
    if (text.tt_len >= maxsize) {
        return 0;
    }
    str[text.tt_len] = '\0';
    return text.tt_len;
}

// Divide value by divisor toward negative infinity.
long __libc_impl_time_floor_div(long value, long divisor)
{
    long quot = value / divisor;

    if (value % divisor < 0) {
        quot--;
    }
    return quot;
}

// Return the remainder of value by divisor that has divisor's sign.
long __libc_impl_time_floor_mod(long value, long divisor)
{
    long rem = value % divisor;

    if (rem < 0) {
        rem += divisor;
    }
    return rem;
}

// Count the days of the year year.
long __libc_impl_time_year_days(long year)
{
    bool leap = (year % __LIBC_IMPL_TIME_YEARS_PER_LEAP == 0 && year % __LIBC_IMPL_TIME_YEARS_PER_CENTURY != 0) || year % __LIBC_IMPL_TIME_YEARS_PER_ERA == 0;

    return leap ? __LIBC_IMPL_TIME_DAYS_PER_YEAR + 1 : __LIBC_IMPL_TIME_DAYS_PER_YEAR;
}

// Count the days from the epoch to the first of the month mon of year.
long __libc_impl_time_month_days(long year, int mon)
{
    long shifted = mon < __LIBC_IMPL_TIME_MARCH ? year - 1 : year;
    int march = mon < __LIBC_IMPL_TIME_MARCH ? mon + __LIBC_IMPL_TIME_MONTHS_PER_YEAR - __LIBC_IMPL_TIME_MARCH : mon - __LIBC_IMPL_TIME_MARCH;
    long era = __libc_impl_time_floor_div(shifted, __LIBC_IMPL_TIME_YEARS_PER_ERA);
    long yoe = shifted - era * __LIBC_IMPL_TIME_YEARS_PER_ERA;
    long doe = yoe * __LIBC_IMPL_TIME_DAYS_PER_YEAR + yoe / __LIBC_IMPL_TIME_YEARS_PER_LEAP - yoe / __LIBC_IMPL_TIME_YEARS_PER_CENTURY + __libc_impl_time_march_days[march];

    return era * __LIBC_IMPL_TIME_DAYS_PER_ERA + doe - __LIBC_IMPL_TIME_ERA_TO_EPOCH;
}

// Break the time value down into tm as a time of the zone zone.
struct tm *__libc_impl_time_break_down(time_t value, struct tm *tm, const char *zone)
{
    long secs = __libc_impl_time_floor_mod(value, __LIBC_IMPL_TIME_SECS_PER_DAY);
    long days = __libc_impl_time_floor_div(value, __LIBC_IMPL_TIME_SECS_PER_DAY);
    long shifted = days + __LIBC_IMPL_TIME_ERA_TO_EPOCH;
    long era = __libc_impl_time_floor_div(shifted, __LIBC_IMPL_TIME_DAYS_PER_ERA);
    long doe = shifted - era * __LIBC_IMPL_TIME_DAYS_PER_ERA;
    long yoe = (doe - doe / (__LIBC_IMPL_TIME_DAYS_PER_YEAR * __LIBC_IMPL_TIME_YEARS_PER_LEAP) + doe / __LIBC_IMPL_TIME_DAYS_PER_CENTURY - doe / (__LIBC_IMPL_TIME_DAYS_PER_ERA - 1)) / __LIBC_IMPL_TIME_DAYS_PER_YEAR;
    long doy = doe - (yoe * __LIBC_IMPL_TIME_DAYS_PER_YEAR + yoe / __LIBC_IMPL_TIME_YEARS_PER_LEAP - yoe / __LIBC_IMPL_TIME_YEARS_PER_CENTURY);
    int march = __LIBC_IMPL_TIME_MONTHS_PER_YEAR - 1;
    int mon;
    long year;

    while (__libc_impl_time_march_days[march] > doy) {
        march--;
    }
    mon = march < __LIBC_IMPL_TIME_MONTHS_PER_YEAR - __LIBC_IMPL_TIME_MARCH ? march + __LIBC_IMPL_TIME_MARCH : march - (__LIBC_IMPL_TIME_MONTHS_PER_YEAR - __LIBC_IMPL_TIME_MARCH);
    year = era * __LIBC_IMPL_TIME_YEARS_PER_ERA + yoe + (mon < __LIBC_IMPL_TIME_MARCH ? 1 : 0);
    if (year - __LIBC_IMPL_TIME_YEAR_BASE < INT_MIN || year - __LIBC_IMPL_TIME_YEAR_BASE > INT_MAX) {
        errno = __LIBC_SYS_EOVERFLOW;
        return NULL;
    }
    tm->tm_sec = (int) (secs % __LIBC_IMPL_TIME_SECS_PER_MIN);
    tm->tm_min = (int) (secs / __LIBC_IMPL_TIME_SECS_PER_MIN % __LIBC_IMPL_TIME_MINS_PER_HOUR);
    tm->tm_hour = (int) (secs / __LIBC_IMPL_TIME_SECS_PER_HOUR);
    tm->tm_mday = (int) (doy - __libc_impl_time_march_days[march] + __LIBC_IMPL_TIME_FIRST_MDAY);
    tm->tm_mon = mon;
    tm->tm_year = (int) (year - __LIBC_IMPL_TIME_YEAR_BASE);
    tm->tm_wday = (int) __libc_impl_time_floor_mod(days + __LIBC_IMPL_TIME_EPOCH_WDAY, __LIBC_IMPL_TIME_DAYS_PER_WEEK);
    tm->tm_yday = (int) (days - __libc_impl_time_month_days(year, __LIBC_IMPL_TIME_JANUARY));
    tm->tm_isdst = 0;
    tm->__libc_tm_gmtoff = 0;
    tm->__libc_tm_zone = zone;
    return tm;
}

// Append the character ch to text.
void __libc_impl_time_put_char(struct __libc_impl_time_text *text, char ch)
{
    if (text->tt_len < text->tt_size) {
        text->tt_str[text->tt_len] = ch;
    }
    text->tt_len++;
}

// Append at most len characters of the string str to text.
void __libc_impl_time_put_string(struct __libc_impl_time_text *text, const char *str, size_t len)
{
    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        __libc_impl_time_put_char(text, str[i]);
    }
}

// Append value to text in at least width characters padded with pad.
void __libc_impl_time_put_number(struct __libc_impl_time_text *text, long value, int width, char pad)
{
    unsigned long mag = value < 0 ? 0UL - (unsigned long) value : (unsigned long) value;
    char digits[__LIBC_IMPL_TIME_NUMBER_SIZE];
    int len = 0;
    int fill;

    do {
        digits[len] = (char) ('0' + mag % __LIBC_IMPL_TIME_NUMBER_BASE);
        len++;
        mag /= __LIBC_IMPL_TIME_NUMBER_BASE;
    } while (mag != 0);
    fill = width - len - (value < 0 ? 1 : 0);
    for (; fill > 0 && pad == ' '; fill--) {
        __libc_impl_time_put_char(text, ' ');
    }
    if (value < 0) {
        __libc_impl_time_put_char(text, '-');
    }
    for (; fill > 0; fill--) {
        __libc_impl_time_put_char(text, pad);
    }
    while (len > 0) {
        len--;
        __libc_impl_time_put_char(text, digits[len]);
    }
}

// Append value to text as printf's %.2d does.
void __libc_impl_time_put_two_digits(struct __libc_impl_time_text *text, int value)
{
    long mag = value;

    if (mag < 0) {
        __libc_impl_time_put_char(text, '-');
        mag = -mag;
    }
    __libc_impl_time_put_number(text, mag, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
}

// Return the name of the weekday wday.
const char *__libc_impl_time_wday_name(int wday)
{
    if (wday < 0 || wday >= __LIBC_IMPL_TIME_DAYS_PER_WEEK) {
        return NULL;
    }
    return __libc_impl_time_wday_names[wday];
}

// Return the name of the month mon.
const char *__libc_impl_time_mon_name(int mon)
{
    if (mon < 0 || mon >= __LIBC_IMPL_TIME_MONTHS_PER_YEAR) {
        return NULL;
    }
    return __libc_impl_time_mon_names[mon];
}

// Append at most len characters of the name name to text.
void __libc_impl_time_put_name(struct __libc_impl_time_text *text, const char *name, size_t len, const char *unknown)
{
    if (name == NULL) {
        __libc_impl_time_put_string(text, unknown, __libc_impl_string_strlen(unknown));
    } else {
        __libc_impl_time_put_string(text, name, len);
    }
}

// Count the days from the start of the ISO 8601 year to the day yday.
long __libc_impl_time_iso_days(long yday, long wday)
{
    return yday - (yday - wday + __LIBC_IMPL_TIME_ISO_WEEK1_WDAY + __LIBC_IMPL_TIME_ISO_SHIFT) % __LIBC_IMPL_TIME_DAYS_PER_WEEK + __LIBC_IMPL_TIME_ISO_WEEK1_WDAY - __LIBC_IMPL_TIME_ISO_START_WDAY;
}

// Find the ISO 8601 year of tm.
long __libc_impl_time_iso_year(const struct tm *tm, long *days)
{
    long year = tm->tm_year + __LIBC_IMPL_TIME_YEAR_BASE;
    long count = __libc_impl_time_iso_days(tm->tm_yday, tm->tm_wday);
    long next;

    if (count < 0) {
        year--;
        count = __libc_impl_time_iso_days(tm->tm_yday + __libc_impl_time_year_days(year), tm->tm_wday);
    } else {
        next = __libc_impl_time_iso_days(tm->tm_yday - __libc_impl_time_year_days(year), tm->tm_wday);
        if (next >= 0) {
            year++;
            count = next;
        }
    }
    *days = count;
    return year;
}

// Return true if strftime knows the conversion conv after the modifier mod.
bool __libc_impl_time_is_conversion(char mod, char conv)
{
    const char *known = __LIBC_IMPL_TIME_CONVERSIONS;

    if (mod == 'E') {
        known = __LIBC_IMPL_TIME_CONVERSIONS_E;
    } else if (mod == 'O') {
        known = __LIBC_IMPL_TIME_CONVERSIONS_O;
    }
    return conv != '\0' && __libc_impl_string_strchr(known, conv) != NULL;
}

// Return the format strftime's conversion conv stands for.
const char *__libc_impl_time_composite(char conv)
{
    switch (conv) {
        case 'c': {
            return "%a %b %e %H:%M:%S %Y";
        } break;
        case 'D':
        case 'x': {
            return "%m/%d/%y";
        } break;
        case 'F': {
            return "%Y-%m-%d";
        } break;
        case 'r': {
            return "%I:%M:%S %p";
        } break;
        case 'R': {
            return "%H:%M";
        } break;
        case 'T':
        case 'X': {
            return "%H:%M:%S";
        } break;
        default: {
            return NULL;
        } break;
    }
}

// Append strftime's conversion conv of tm to text.
void __libc_impl_time_put_conversion(struct __libc_impl_time_text *text, char conv, const struct tm *tm)
{
    long year = tm->tm_year + __LIBC_IMPL_TIME_YEAR_BASE;
    int hour = tm->tm_hour;
    long days;
    long mins;

    switch (conv) {
        case 'a': {
            __libc_impl_time_put_name(text, __libc_impl_time_wday_name(tm->tm_wday), __LIBC_IMPL_TIME_ABBREV_LEN, __LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'A': {
            __libc_impl_time_put_name(text, __libc_impl_time_wday_name(tm->tm_wday), SIZE_MAX, __LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'b':
        case 'h': {
            __libc_impl_time_put_name(text, __libc_impl_time_mon_name(tm->tm_mon), __LIBC_IMPL_TIME_ABBREV_LEN, __LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'B': {
            __libc_impl_time_put_name(text, __libc_impl_time_mon_name(tm->tm_mon), SIZE_MAX, __LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'C': {
            __libc_impl_time_put_number(text, __libc_impl_time_floor_div(year, __LIBC_IMPL_TIME_YEARS_PER_CENTURY), __LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'd': {
            __libc_impl_time_put_number(text, tm->tm_mday, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'e': {
            __libc_impl_time_put_number(text, tm->tm_mday, __LIBC_IMPL_TIME_WIDTH_FIELD, ' ');
        } break;
        case 'g': {
            __libc_impl_time_put_number(text, __libc_impl_time_floor_mod(__libc_impl_time_iso_year(tm, &days), __LIBC_IMPL_TIME_YEARS_PER_CENTURY), __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'G': {
            __libc_impl_time_put_number(text, __libc_impl_time_iso_year(tm, &days), __LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'H': {
            __libc_impl_time_put_number(text, tm->tm_hour, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'I': {
            if (hour > __LIBC_IMPL_TIME_HOURS_PER_HALF) {
                hour -= __LIBC_IMPL_TIME_HOURS_PER_HALF;
            } else if (hour == 0) {
                hour = __LIBC_IMPL_TIME_HOURS_PER_HALF;
            }
            __libc_impl_time_put_number(text, hour, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'j': {
            __libc_impl_time_put_number(text, tm->tm_yday + 1L, __LIBC_IMPL_TIME_WIDTH_YDAY, '0');
        } break;
        case 'm': {
            __libc_impl_time_put_number(text, tm->tm_mon + 1L, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'M': {
            __libc_impl_time_put_number(text, tm->tm_min, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'n': {
            __libc_impl_time_put_char(text, '\n');
        } break;
        case 'p': {
            __libc_impl_time_put_string(text, hour >= __LIBC_IMPL_TIME_HOURS_PER_HALF ? "PM" : "AM", SIZE_MAX);
        } break;
        case 'S': {
            __libc_impl_time_put_number(text, tm->tm_sec, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 't': {
            __libc_impl_time_put_char(text, '\t');
        } break;
        case 'u': {
            __libc_impl_time_put_number(text, (tm->tm_wday - 1L + __LIBC_IMPL_TIME_DAYS_PER_WEEK) % __LIBC_IMPL_TIME_DAYS_PER_WEEK + 1, __LIBC_IMPL_TIME_WIDTH_WDAY, '0');
        } break;
        case 'U': {
            __libc_impl_time_put_number(text, (tm->tm_yday - (long) tm->tm_wday + __LIBC_IMPL_TIME_DAYS_PER_WEEK) / __LIBC_IMPL_TIME_DAYS_PER_WEEK, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'V': {
            __libc_impl_time_iso_year(tm, &days);
            __libc_impl_time_put_number(text, days / __LIBC_IMPL_TIME_DAYS_PER_WEEK + 1, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'w': {
            __libc_impl_time_put_number(text, tm->tm_wday, __LIBC_IMPL_TIME_WIDTH_WDAY, '0');
        } break;
        case 'W': {
            __libc_impl_time_put_number(text, (tm->tm_yday - (tm->tm_wday - 1L + __LIBC_IMPL_TIME_DAYS_PER_WEEK) % __LIBC_IMPL_TIME_DAYS_PER_WEEK + __LIBC_IMPL_TIME_DAYS_PER_WEEK) / __LIBC_IMPL_TIME_DAYS_PER_WEEK, __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'y': {
            __libc_impl_time_put_number(text, __libc_impl_time_floor_mod(year, __LIBC_IMPL_TIME_YEARS_PER_CENTURY), __LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'Y': {
            __libc_impl_time_put_number(text, year, __LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'z': {
            mins = tm->__libc_tm_gmtoff / __LIBC_IMPL_TIME_SECS_PER_MIN;
            __libc_impl_time_put_char(text, tm->__libc_tm_gmtoff < 0 ? '-' : '+');
            if (mins < 0) {
                mins = -mins;
            }
            __libc_impl_time_put_number(text, mins / __LIBC_IMPL_TIME_MINS_PER_HOUR * __LIBC_IMPL_TIME_OFFSET_HOUR + mins % __LIBC_IMPL_TIME_MINS_PER_HOUR, __LIBC_IMPL_TIME_WIDTH_OFFSET, '0');
        } break;
        case 'Z': {
            __libc_impl_time_put_string(text, tm->__libc_tm_zone != NULL ? tm->__libc_tm_zone : __LIBC_IMPL_TIME_ZONE_UTC, SIZE_MAX);
        } break;
        default: {
            __libc_impl_time_put_char(text, '%');
        } break;
    }
}

// Append the conversions of tm that format names to text.
void __libc_impl_time_put_format(struct __libc_impl_time_text *text, const char *format, const struct tm *tm)
{
    const char *ptr = format;

    while (*ptr != '\0') {
        const char *start = ptr;
        const char *composite;
        char mod = '\0';

        if (*ptr != '%') {
            __libc_impl_time_put_char(text, *ptr);
            ptr++;
            continue;
        }
        ptr++;
        if (*ptr == 'E' || *ptr == 'O') {
            mod = *ptr;
            ptr++;
        }
        if (! __libc_impl_time_is_conversion(mod, *ptr)) {
            if (*ptr != '\0') {
                ptr++;
            }
            __libc_impl_time_put_string(text, start, (size_t) (ptr - start));
            continue;
        }
        composite = __libc_impl_time_composite(*ptr);
        if (composite != NULL) {
            __libc_impl_time_put_format(text, composite, tm);
        } else {
            __libc_impl_time_put_conversion(text, *ptr, tm);
        }
        ptr++;
    }
}
