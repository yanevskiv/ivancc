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
#include <time.h>

// The range of a broken-down year.
#include <limits.h>

// The length of a name written whole.
#include <stdint.h>

// The error number of a time out of range.
#include <errno.h>

// The conversions strftime knows.
#include <string.h>

// The clocks clock and time read.
#include <_sys.h>

// The days before each month of a year that starts in March.
const long _Time_MarchDays[_TIME_MONTHS_PER_YEAR] = { 0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337 };

// The names of the weekdays.
const char *const _Time_WdayNames[_TIME_DAYS_PER_WEEK] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// The names of the months.
const char *const _Time_MonNames[_TIME_MONTHS_PER_YEAR] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

// The broken-down time gmtime and localtime return.
struct tm _Time_Tm;

// The text asctime and ctime return.
char _Time_AsctimeText[_TIME_ASCTIME_SIZE];

// Divide value by divisor toward negative infinity.
long _Time_FloorDiv(long value, long divisor)
{
    long quot = value / divisor;

    if (value % divisor < 0) {
        quot--;
    }
    return quot;
}

// Return the remainder of value by divisor that has divisor's sign.
long _Time_FloorMod(long value, long divisor)
{
    long rem = value % divisor;

    if (rem < 0) {
        rem += divisor;
    }
    return rem;
}

// Count the days of the year year.
long _Time_YearDays(long year)
{
    _Bool leap = (year % _TIME_YEARS_PER_LEAP == 0 && year % _TIME_YEARS_PER_CENTURY != 0) || year % _TIME_YEARS_PER_ERA == 0;

    return leap ? _TIME_DAYS_PER_YEAR + 1 : _TIME_DAYS_PER_YEAR;
}

// Count the days from the epoch to the first of the month mon of year.
long _Time_MonthDays(long year, int mon)
{
    long shifted = mon < _TIME_MARCH ? year - 1 : year;
    int march = mon < _TIME_MARCH ? mon + _TIME_MONTHS_PER_YEAR - _TIME_MARCH : mon - _TIME_MARCH;
    long era = _Time_FloorDiv(shifted, _TIME_YEARS_PER_ERA);
    long yoe = shifted - era * _TIME_YEARS_PER_ERA;
    long doe = yoe * _TIME_DAYS_PER_YEAR + yoe / _TIME_YEARS_PER_LEAP - yoe / _TIME_YEARS_PER_CENTURY + _Time_MarchDays[march];

    return era * _TIME_DAYS_PER_ERA + doe - _TIME_ERA_TO_EPOCH;
}

// Break the time value down into tm as a time of the zone zone.
struct tm *_Time_BreakDown(time_t value, struct tm *tm, const char *zone)
{
    long secs = _Time_FloorMod(value, _TIME_SECS_PER_DAY);
    long days = _Time_FloorDiv(value, _TIME_SECS_PER_DAY);
    long shifted = days + _TIME_ERA_TO_EPOCH;
    long era = _Time_FloorDiv(shifted, _TIME_DAYS_PER_ERA);
    long doe = shifted - era * _TIME_DAYS_PER_ERA;
    long yoe = (doe - doe / (_TIME_DAYS_PER_YEAR * _TIME_YEARS_PER_LEAP) + doe / _TIME_DAYS_PER_CENTURY - doe / (_TIME_DAYS_PER_ERA - 1)) / _TIME_DAYS_PER_YEAR;
    long doy = doe - (yoe * _TIME_DAYS_PER_YEAR + yoe / _TIME_YEARS_PER_LEAP - yoe / _TIME_YEARS_PER_CENTURY);
    int march = _TIME_MONTHS_PER_YEAR - 1;
    int mon;
    long year;

    while (_Time_MarchDays[march] > doy) {
        march--;
    }
    mon = march < _TIME_MONTHS_PER_YEAR - _TIME_MARCH ? march + _TIME_MARCH : march - (_TIME_MONTHS_PER_YEAR - _TIME_MARCH);
    year = era * _TIME_YEARS_PER_ERA + yoe + (mon < _TIME_MARCH ? 1 : 0);
    if (year - _TIME_YEAR_BASE < INT_MIN || year - _TIME_YEAR_BASE > INT_MAX) {
        errno = _SYS_EOVERFLOW;
        return NULL;
    }
    tm->tm_sec = (int) (secs % _TIME_SECS_PER_MIN);
    tm->tm_min = (int) (secs / _TIME_SECS_PER_MIN % _TIME_MINS_PER_HOUR);
    tm->tm_hour = (int) (secs / _TIME_SECS_PER_HOUR);
    tm->tm_mday = (int) (doy - _Time_MarchDays[march] + _TIME_FIRST_MDAY);
    tm->tm_mon = mon;
    tm->tm_year = (int) (year - _TIME_YEAR_BASE);
    tm->tm_wday = (int) _Time_FloorMod(days + _TIME_EPOCH_WDAY, _TIME_DAYS_PER_WEEK);
    tm->tm_yday = (int) (days - _Time_MonthDays(year, _TIME_JANUARY));
    tm->tm_isdst = 0;
    tm->_Time_TmGmtoff = 0;
    tm->_Time_TmZone = zone;
    return tm;
}

// Append the character ch to text.
void _Time_PutChar(struct _Time_Text *text, char ch)
{
    if (text->tt_len < text->tt_size) {
        text->tt_str[text->tt_len] = ch;
    }
    text->tt_len++;
}

// Append at most len characters of the string str to text.
void _Time_PutString(struct _Time_Text *text, const char *str, size_t len)
{
    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        _Time_PutChar(text, str[i]);
    }
}

// Append value to text in at least width characters padded with pad.
void _Time_PutNumber(struct _Time_Text *text, long value, int width, char pad)
{
    unsigned long mag = value < 0 ? 0UL - (unsigned long) value : (unsigned long) value;
    char digits[_TIME_NUMBER_SIZE];
    int len = 0;
    int fill;

    do {
        digits[len] = (char) ('0' + mag % _TIME_NUMBER_BASE);
        len++;
        mag /= _TIME_NUMBER_BASE;
    } while (mag != 0);
    fill = width - len - (value < 0 ? 1 : 0);
    for (; fill > 0 && pad == ' '; fill--) {
        _Time_PutChar(text, ' ');
    }
    if (value < 0) {
        _Time_PutChar(text, '-');
    }
    for (; fill > 0; fill--) {
        _Time_PutChar(text, pad);
    }
    while (len > 0) {
        len--;
        _Time_PutChar(text, digits[len]);
    }
}

// Append value to text as printf's %.2d does.
void _Time_PutTwoDigits(struct _Time_Text *text, int value)
{
    long mag = value;

    if (mag < 0) {
        _Time_PutChar(text, '-');
        mag = -mag;
    }
    _Time_PutNumber(text, mag, _TIME_WIDTH_FIELD, '0');
}

// Return the name of the weekday wday.
const char *_Time_WdayName(int wday)
{
    if (wday < 0 || wday >= _TIME_DAYS_PER_WEEK) {
        return NULL;
    }
    return _Time_WdayNames[wday];
}

// Return the name of the month mon.
const char *_Time_MonName(int mon)
{
    if (mon < 0 || mon >= _TIME_MONTHS_PER_YEAR) {
        return NULL;
    }
    return _Time_MonNames[mon];
}

// Append at most len characters of the name name to text.
void _Time_PutName(struct _Time_Text *text, const char *name, size_t len, const char *unknown)
{
    if (name == NULL) {
        _Time_PutString(text, unknown, strlen(unknown));
    } else {
        _Time_PutString(text, name, len);
    }
}

// Count the days from the start of the ISO 8601 year to the day yday.
long _Time_IsoDays(long yday, long wday)
{
    return yday - (yday - wday + _TIME_ISO_WEEK1_WDAY + _TIME_ISO_SHIFT) % _TIME_DAYS_PER_WEEK + _TIME_ISO_WEEK1_WDAY - _TIME_ISO_START_WDAY;
}

// Find the ISO 8601 year of tm.
long _Time_IsoYear(const struct tm *tm, long *days)
{
    long year = tm->tm_year + _TIME_YEAR_BASE;
    long count = _Time_IsoDays(tm->tm_yday, tm->tm_wday);
    long next;

    if (count < 0) {
        year--;
        count = _Time_IsoDays(tm->tm_yday + _Time_YearDays(year), tm->tm_wday);
    } else {
        next = _Time_IsoDays(tm->tm_yday - _Time_YearDays(year), tm->tm_wday);
        if (next >= 0) {
            year++;
            count = next;
        }
    }
    *days = count;
    return year;
}

// Return true if strftime knows the conversion conv after the modifier mod.
_Bool _Time_IsConversion(char mod, char conv)
{
    const char *known = _TIME_CONVERSIONS;

    if (mod == 'E') {
        known = _TIME_CONVERSIONS_E;
    } else if (mod == 'O') {
        known = _TIME_CONVERSIONS_O;
    }
    return conv != '\0' && strchr(known, conv) != NULL;
}

// Return the format strftime's conversion conv stands for.
const char *_Time_Composite(char conv)
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
void _Time_PutConversion(struct _Time_Text *text, char conv, const struct tm *tm)
{
    long year = tm->tm_year + _TIME_YEAR_BASE;
    int hour = tm->tm_hour;
    long days;
    long mins;

    switch (conv) {
        case 'a': {
            _Time_PutName(text, _Time_WdayName(tm->tm_wday), _TIME_ABBREV_LEN, _TIME_UNKNOWN_NAME);
        } break;
        case 'A': {
            _Time_PutName(text, _Time_WdayName(tm->tm_wday), SIZE_MAX, _TIME_UNKNOWN_NAME);
        } break;
        case 'b':
        case 'h': {
            _Time_PutName(text, _Time_MonName(tm->tm_mon), _TIME_ABBREV_LEN, _TIME_UNKNOWN_NAME);
        } break;
        case 'B': {
            _Time_PutName(text, _Time_MonName(tm->tm_mon), SIZE_MAX, _TIME_UNKNOWN_NAME);
        } break;
        case 'C': {
            _Time_PutNumber(text, _Time_FloorDiv(year, _TIME_YEARS_PER_CENTURY), _TIME_WIDTH_YEAR, '0');
        } break;
        case 'd': {
            _Time_PutNumber(text, tm->tm_mday, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'e': {
            _Time_PutNumber(text, tm->tm_mday, _TIME_WIDTH_FIELD, ' ');
        } break;
        case 'g': {
            _Time_PutNumber(text, _Time_FloorMod(_Time_IsoYear(tm, &days), _TIME_YEARS_PER_CENTURY), _TIME_WIDTH_FIELD, '0');
        } break;
        case 'G': {
            _Time_PutNumber(text, _Time_IsoYear(tm, &days), _TIME_WIDTH_YEAR, '0');
        } break;
        case 'H': {
            _Time_PutNumber(text, tm->tm_hour, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'I': {
            if (hour > _TIME_HOURS_PER_HALF) {
                hour -= _TIME_HOURS_PER_HALF;
            } else if (hour == 0) {
                hour = _TIME_HOURS_PER_HALF;
            }
            _Time_PutNumber(text, hour, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'j': {
            _Time_PutNumber(text, tm->tm_yday + 1L, _TIME_WIDTH_YDAY, '0');
        } break;
        case 'm': {
            _Time_PutNumber(text, tm->tm_mon + 1L, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'M': {
            _Time_PutNumber(text, tm->tm_min, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'n': {
            _Time_PutChar(text, '\n');
        } break;
        case 'p': {
            _Time_PutString(text, hour >= _TIME_HOURS_PER_HALF ? "PM" : "AM", SIZE_MAX);
        } break;
        case 'S': {
            _Time_PutNumber(text, tm->tm_sec, _TIME_WIDTH_FIELD, '0');
        } break;
        case 't': {
            _Time_PutChar(text, '\t');
        } break;
        case 'u': {
            _Time_PutNumber(text, (tm->tm_wday - 1L + _TIME_DAYS_PER_WEEK) % _TIME_DAYS_PER_WEEK + 1, _TIME_WIDTH_WDAY, '0');
        } break;
        case 'U': {
            _Time_PutNumber(text, (tm->tm_yday - (long) tm->tm_wday + _TIME_DAYS_PER_WEEK) / _TIME_DAYS_PER_WEEK, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'V': {
            _Time_IsoYear(tm, &days);
            _Time_PutNumber(text, days / _TIME_DAYS_PER_WEEK + 1, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'w': {
            _Time_PutNumber(text, tm->tm_wday, _TIME_WIDTH_WDAY, '0');
        } break;
        case 'W': {
            _Time_PutNumber(text, (tm->tm_yday - (tm->tm_wday - 1L + _TIME_DAYS_PER_WEEK) % _TIME_DAYS_PER_WEEK + _TIME_DAYS_PER_WEEK) / _TIME_DAYS_PER_WEEK, _TIME_WIDTH_FIELD, '0');
        } break;
        case 'y': {
            _Time_PutNumber(text, _Time_FloorMod(year, _TIME_YEARS_PER_CENTURY), _TIME_WIDTH_FIELD, '0');
        } break;
        case 'Y': {
            _Time_PutNumber(text, year, _TIME_WIDTH_YEAR, '0');
        } break;
        case 'z': {
            mins = tm->_Time_TmGmtoff / _TIME_SECS_PER_MIN;
            _Time_PutChar(text, tm->_Time_TmGmtoff < 0 ? '-' : '+');
            if (mins < 0) {
                mins = -mins;
            }
            _Time_PutNumber(text, mins / _TIME_MINS_PER_HOUR * _TIME_OFFSET_HOUR + mins % _TIME_MINS_PER_HOUR, _TIME_WIDTH_OFFSET, '0');
        } break;
        case 'Z': {
            _Time_PutString(text, tm->_Time_TmZone != NULL ? tm->_Time_TmZone : _TIME_ZONE_UTC, SIZE_MAX);
        } break;
        default: {
            _Time_PutChar(text, '%');
        } break;
    }
}

// Append the conversions of tm that format names to text.
void _Time_PutFormat(struct _Time_Text *text, const char *format, const struct tm *tm)
{
    const char *ptr = format;

    while (*ptr != '\0') {
        const char *start = ptr;
        const char *composite;
        char mod = '\0';

        if (*ptr != '%') {
            _Time_PutChar(text, *ptr);
            ptr++;
            continue;
        }
        ptr++;
        if (*ptr == 'E' || *ptr == 'O') {
            mod = *ptr;
            ptr++;
        }
        if (! _Time_IsConversion(mod, *ptr)) {
            if (*ptr != '\0') {
                ptr++;
            }
            _Time_PutString(text, start, (size_t) (ptr - start));
            continue;
        }
        composite = _Time_Composite(*ptr);
        if (composite != NULL) {
            _Time_PutFormat(text, composite, tm);
        } else {
            _Time_PutConversion(text, *ptr, tm);
        }
        ptr++;
    }
}

// Return the processor time the program has used.
clock_t clock(void)
{
    struct _Sys_Timespec spec;

    if (_Sys_ClockGettime(_SYS_CLOCK_PROCESS_CPUTIME_ID, &spec) != 0) {
        return (clock_t) -1;
    }
    return spec.tv_sec * CLOCKS_PER_SEC + spec.tv_nsec / _TIME_NSECS_PER_CLOCK;
}

// Return the seconds from time0 to time1.
double difftime(time_t time1, time_t time0)
{
    if (time1 >= time0) {
        return (double) ((unsigned long) time1 - (unsigned long) time0);
    }
    return -(double) ((unsigned long) time0 - (unsigned long) time1);
}

// Convert the local time timeptr to a calendar time.
time_t mktime(struct tm *timeptr)
{
    long year = timeptr->tm_year + _TIME_YEAR_BASE + _Time_FloorDiv(timeptr->tm_mon, _TIME_MONTHS_PER_YEAR);
    int mon = (int) _Time_FloorMod(timeptr->tm_mon, _TIME_MONTHS_PER_YEAR);
    long days = _Time_MonthDays(year, mon) + timeptr->tm_mday - _TIME_FIRST_MDAY;
    time_t value = days * _TIME_SECS_PER_DAY + timeptr->tm_hour * _TIME_SECS_PER_HOUR + timeptr->tm_min * _TIME_SECS_PER_MIN + timeptr->tm_sec;
    struct tm result;

    if (_Time_BreakDown(value, &result, _TIME_ZONE_UTC) == NULL) {
        return (time_t) -1;
    }
    *timeptr = result;
    return value;
}

// Return the current calendar time.
time_t time(time_t *timer)
{
    struct _Sys_Timespec spec;
    time_t value = (time_t) -1;

    if (_Sys_ClockGettime(_SYS_CLOCK_REALTIME, &spec) == 0) {
        value = spec.tv_sec;
    }
    if (timer != NULL) {
        *timer = value;
    }
    return value;
}

// Write the broken-down time timeptr as text.
char *asctime(const struct tm *timeptr)
{
    struct _Time_Text text = {
        .tt_str  = _Time_AsctimeText,
        .tt_size = _TIME_ASCTIME_SIZE,
        .tt_len  = 0
    };

    if (timeptr->tm_year > INT_MAX - _TIME_YEAR_BASE) {
        errno = _SYS_EOVERFLOW;
        return NULL;
    }
    _Time_PutName(&text, _Time_WdayName(timeptr->tm_wday), _TIME_ABBREV_LEN, _TIME_UNKNOWN_ABBREV);
    _Time_PutChar(&text, ' ');
    _Time_PutName(&text, _Time_MonName(timeptr->tm_mon), _TIME_ABBREV_LEN, _TIME_UNKNOWN_ABBREV);
    _Time_PutNumber(&text, timeptr->tm_mday, _TIME_WIDTH_MDAY, ' ');
    _Time_PutChar(&text, ' ');
    _Time_PutTwoDigits(&text, timeptr->tm_hour);
    _Time_PutChar(&text, ':');
    _Time_PutTwoDigits(&text, timeptr->tm_min);
    _Time_PutChar(&text, ':');
    _Time_PutTwoDigits(&text, timeptr->tm_sec);
    _Time_PutChar(&text, ' ');
    _Time_PutNumber(&text, timeptr->tm_year + _TIME_YEAR_BASE, _TIME_WIDTH_YEAR, '0');
    _Time_PutChar(&text, '\n');
    _Time_PutChar(&text, '\0');
    return _Time_AsctimeText;
}

// Write the local time of timer as text.
char *ctime(const time_t *timer)
{
    struct tm *tm = localtime(timer);

    if (tm == NULL) {
        return NULL;
    }
    return asctime(tm);
}

// Break the time timer down as a time of UTC.
struct tm *gmtime(const time_t *timer)
{
    return _Time_BreakDown(*timer, &_Time_Tm, _TIME_ZONE_GMT);
}

// Break the time timer down as a local time.
struct tm *localtime(const time_t *timer)
{
    return _Time_BreakDown(*timer, &_Time_Tm, _TIME_ZONE_UTC);
}

// Write the conversions of timeptr that format names into str.
size_t strftime(char *restrict str, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr)
{
    struct _Time_Text text = {
        .tt_str  = str,
        .tt_size = maxsize,
        .tt_len  = 0
    };

    _Time_PutFormat(&text, format, timeptr);
    if (text.tt_len >= maxsize) {
        return 0;
    }
    str[text.tt_len] = '\0';
    return text.tt_len;
}
