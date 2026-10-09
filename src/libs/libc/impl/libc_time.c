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
#include <libc/impl/libc_limits.h>

// The length of a name written whole.
#include <libc/impl/libc_stdint.h>

// The error number of a time out of range.
#include <libc/impl/libc_errno.h>

// The conversions strftime knows.
#include <libc/impl/libc_string.h>

// The clocks clock and time read.
#include <libc/libc_sys.h>

// The days before each month of a year that starts in March.
static const long _Libc_Impl_Time_MarchDays[_LIBC_IMPL_TIME_MONTHS_PER_YEAR] = { 0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337 };

// The names of the weekdays.
static const char *const _Libc_Impl_Time_WdayNames[_LIBC_IMPL_TIME_DAYS_PER_WEEK] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// The names of the months.
static const char *const _Libc_Impl_Time_MonNames[_LIBC_IMPL_TIME_MONTHS_PER_YEAR] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

// The broken-down time gmtime and localtime return.
static struct tm _Libc_Impl_Time_Tm;

// The text asctime and ctime return.
static char _Libc_Impl_Time_AsctimeText[_LIBC_IMPL_TIME_ASCTIME_SIZE];

// Return the processor time the program has used.
_Libc_Impl_Time_clock_t _Libc_Impl_Time_clock(void)
{
    struct _Libc_Sys_timespec spec;

    if (_Libc_Sys_clock_gettime(_LIBC_SYS_CLOCK_PROCESS_CPUTIME_ID, &spec) != 0) {
        return (_Libc_Impl_Time_clock_t) -1;
    }
    return spec.tv_sec * _LIBC_IMPL_TIME_CLOCKS_PER_SEC + spec.tv_nsec / _LIBC_IMPL_TIME_NSECS_PER_CLOCK;
}

// Return the seconds from time0 to time1.
double _Libc_Impl_Time_difftime(_Libc_Impl_Time_time_t time1, _Libc_Impl_Time_time_t time0)
{
    if (time1 >= time0) {
        return (double) ((unsigned long) time1 - (unsigned long) time0);
    }
    return -(double) ((unsigned long) time0 - (unsigned long) time1);
}

// Convert the local time timeptr to a calendar time.
_Libc_Impl_Time_time_t _Libc_Impl_Time_mktime(struct tm *timeptr)
{
    long year = timeptr->tm_year + _LIBC_IMPL_TIME_YEAR_BASE + _Libc_Impl_Time_FloorDiv(timeptr->tm_mon, _LIBC_IMPL_TIME_MONTHS_PER_YEAR);
    int mon = (int) _Libc_Impl_Time_FloorMod(timeptr->tm_mon, _LIBC_IMPL_TIME_MONTHS_PER_YEAR);
    long days = _Libc_Impl_Time_MonthDays(year, mon) + timeptr->tm_mday - _LIBC_IMPL_TIME_FIRST_MDAY;
    _Libc_Impl_Time_time_t value = days * _LIBC_IMPL_TIME_SECS_PER_DAY + timeptr->tm_hour * _LIBC_IMPL_TIME_SECS_PER_HOUR + timeptr->tm_min * _LIBC_IMPL_TIME_SECS_PER_MIN + timeptr->tm_sec;
    struct tm result;

    if (_Libc_Impl_Time_BreakDown(value, &result, _LIBC_IMPL_TIME_ZONE_UTC) == _LIBC_IMPL_STDDEF_NULL) {
        return (_Libc_Impl_Time_time_t) -1;
    }
    *timeptr = result;
    return value;
}

// Return the current calendar time.
_Libc_Impl_Time_time_t _Libc_Impl_Time_time(_Libc_Impl_Time_time_t *timer)
{
    struct _Libc_Sys_timespec spec;
    _Libc_Impl_Time_time_t value = (_Libc_Impl_Time_time_t) -1;

    if (_Libc_Sys_clock_gettime(_LIBC_SYS_CLOCK_REALTIME, &spec) == 0) {
        value = spec.tv_sec;
    }
    if (timer != _LIBC_IMPL_STDDEF_NULL) {
        *timer = value;
    }
    return value;
}

// Write the broken-down time timeptr as text.
char *_Libc_Impl_Time_asctime(const struct tm *timeptr)
{
    struct _Libc_Impl_Time_Text text = {
        .tt_str  = _Libc_Impl_Time_AsctimeText,
        .tt_size = _LIBC_IMPL_TIME_ASCTIME_SIZE,
        .tt_len  = 0
    };

    if (timeptr->tm_year > _LIBC_IMPL_LIMITS_INT_MAX - _LIBC_IMPL_TIME_YEAR_BASE) {
        errno = _LIBC_SYS_EOVERFLOW;
        return _LIBC_IMPL_STDDEF_NULL;
    }
    _Libc_Impl_Time_PutName(&text, _Libc_Impl_Time_WdayName(timeptr->tm_wday), _LIBC_IMPL_TIME_ABBREV_LEN, _LIBC_IMPL_TIME_UNKNOWN_ABBREV);
    _Libc_Impl_Time_PutChar(&text, ' ');
    _Libc_Impl_Time_PutName(&text, _Libc_Impl_Time_MonName(timeptr->tm_mon), _LIBC_IMPL_TIME_ABBREV_LEN, _LIBC_IMPL_TIME_UNKNOWN_ABBREV);
    _Libc_Impl_Time_PutNumber(&text, timeptr->tm_mday, _LIBC_IMPL_TIME_WIDTH_MDAY, ' ');
    _Libc_Impl_Time_PutChar(&text, ' ');
    _Libc_Impl_Time_PutTwoDigits(&text, timeptr->tm_hour);
    _Libc_Impl_Time_PutChar(&text, ':');
    _Libc_Impl_Time_PutTwoDigits(&text, timeptr->tm_min);
    _Libc_Impl_Time_PutChar(&text, ':');
    _Libc_Impl_Time_PutTwoDigits(&text, timeptr->tm_sec);
    _Libc_Impl_Time_PutChar(&text, ' ');
    _Libc_Impl_Time_PutNumber(&text, timeptr->tm_year + _LIBC_IMPL_TIME_YEAR_BASE, _LIBC_IMPL_TIME_WIDTH_YEAR, '0');
    _Libc_Impl_Time_PutChar(&text, '\n');
    _Libc_Impl_Time_PutChar(&text, '\0');
    return _Libc_Impl_Time_AsctimeText;
}

// Write the local time of timer as text.
char *_Libc_Impl_Time_ctime(const _Libc_Impl_Time_time_t *timer)
{
    struct tm *tm = _Libc_Impl_Time_localtime(timer);

    if (tm == _LIBC_IMPL_STDDEF_NULL) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    return _Libc_Impl_Time_asctime(tm);
}

// Break the time timer down as a time of UTC.
struct tm *_Libc_Impl_Time_gmtime(const _Libc_Impl_Time_time_t *timer)
{
    return _Libc_Impl_Time_BreakDown(*timer, &_Libc_Impl_Time_Tm, _LIBC_IMPL_TIME_ZONE_GMT);
}

// Break the time timer down as a local time.
struct tm *_Libc_Impl_Time_localtime(const _Libc_Impl_Time_time_t *timer)
{
    return _Libc_Impl_Time_BreakDown(*timer, &_Libc_Impl_Time_Tm, _LIBC_IMPL_TIME_ZONE_UTC);
}

// Write the conversions of timeptr that format names into str.
_Libc_Impl_Stddef_size_t _Libc_Impl_Time_strftime(char *restrict str, _Libc_Impl_Stddef_size_t maxsize, const char *restrict format, const struct tm *restrict timeptr)
{
    struct _Libc_Impl_Time_Text text = {
        .tt_str  = str,
        .tt_size = maxsize,
        .tt_len  = 0
    };

    _Libc_Impl_Time_PutFormat(&text, format, timeptr);
    if (text.tt_len >= maxsize) {
        return 0;
    }
    str[text.tt_len] = '\0';
    return text.tt_len;
}

// Divide value by divisor toward negative infinity.
long _Libc_Impl_Time_FloorDiv(long value, long divisor)
{
    long quot = value / divisor;

    if (value % divisor < 0) {
        quot--;
    }
    return quot;
}

// Return the remainder of value by divisor that has divisor's sign.
long _Libc_Impl_Time_FloorMod(long value, long divisor)
{
    long rem = value % divisor;

    if (rem < 0) {
        rem += divisor;
    }
    return rem;
}

// Count the days of the year year.
long _Libc_Impl_Time_YearDays(long year)
{
    _Bool leap = (year % _LIBC_IMPL_TIME_YEARS_PER_LEAP == 0 && year % _LIBC_IMPL_TIME_YEARS_PER_CENTURY != 0) || year % _LIBC_IMPL_TIME_YEARS_PER_ERA == 0;

    return leap ? _LIBC_IMPL_TIME_DAYS_PER_YEAR + 1 : _LIBC_IMPL_TIME_DAYS_PER_YEAR;
}

// Count the days from the epoch to the first of the month mon of year.
long _Libc_Impl_Time_MonthDays(long year, int mon)
{
    long shifted = mon < _LIBC_IMPL_TIME_MARCH ? year - 1 : year;
    int march = mon < _LIBC_IMPL_TIME_MARCH ? mon + _LIBC_IMPL_TIME_MONTHS_PER_YEAR - _LIBC_IMPL_TIME_MARCH : mon - _LIBC_IMPL_TIME_MARCH;
    long era = _Libc_Impl_Time_FloorDiv(shifted, _LIBC_IMPL_TIME_YEARS_PER_ERA);
    long yoe = shifted - era * _LIBC_IMPL_TIME_YEARS_PER_ERA;
    long doe = yoe * _LIBC_IMPL_TIME_DAYS_PER_YEAR + yoe / _LIBC_IMPL_TIME_YEARS_PER_LEAP - yoe / _LIBC_IMPL_TIME_YEARS_PER_CENTURY + _Libc_Impl_Time_MarchDays[march];

    return era * _LIBC_IMPL_TIME_DAYS_PER_ERA + doe - _LIBC_IMPL_TIME_ERA_TO_EPOCH;
}

// Break the time value down into tm as a time of the zone zone.
struct tm *_Libc_Impl_Time_BreakDown(_Libc_Impl_Time_time_t value, struct tm *tm, const char *zone)
{
    long secs = _Libc_Impl_Time_FloorMod(value, _LIBC_IMPL_TIME_SECS_PER_DAY);
    long days = _Libc_Impl_Time_FloorDiv(value, _LIBC_IMPL_TIME_SECS_PER_DAY);
    long shifted = days + _LIBC_IMPL_TIME_ERA_TO_EPOCH;
    long era = _Libc_Impl_Time_FloorDiv(shifted, _LIBC_IMPL_TIME_DAYS_PER_ERA);
    long doe = shifted - era * _LIBC_IMPL_TIME_DAYS_PER_ERA;
    long yoe = (doe - doe / (_LIBC_IMPL_TIME_DAYS_PER_YEAR * _LIBC_IMPL_TIME_YEARS_PER_LEAP) + doe / _LIBC_IMPL_TIME_DAYS_PER_CENTURY - doe / (_LIBC_IMPL_TIME_DAYS_PER_ERA - 1)) / _LIBC_IMPL_TIME_DAYS_PER_YEAR;
    long doy = doe - (yoe * _LIBC_IMPL_TIME_DAYS_PER_YEAR + yoe / _LIBC_IMPL_TIME_YEARS_PER_LEAP - yoe / _LIBC_IMPL_TIME_YEARS_PER_CENTURY);
    int march = _LIBC_IMPL_TIME_MONTHS_PER_YEAR - 1;
    int mon;
    long year;

    while (_Libc_Impl_Time_MarchDays[march] > doy) {
        march--;
    }
    mon = march < _LIBC_IMPL_TIME_MONTHS_PER_YEAR - _LIBC_IMPL_TIME_MARCH ? march + _LIBC_IMPL_TIME_MARCH : march - (_LIBC_IMPL_TIME_MONTHS_PER_YEAR - _LIBC_IMPL_TIME_MARCH);
    year = era * _LIBC_IMPL_TIME_YEARS_PER_ERA + yoe + (mon < _LIBC_IMPL_TIME_MARCH ? 1 : 0);
    if (year - _LIBC_IMPL_TIME_YEAR_BASE < _LIBC_IMPL_LIMITS_INT_MIN || year - _LIBC_IMPL_TIME_YEAR_BASE > _LIBC_IMPL_LIMITS_INT_MAX) {
        errno = _LIBC_SYS_EOVERFLOW;
        return _LIBC_IMPL_STDDEF_NULL;
    }
    tm->tm_sec = (int) (secs % _LIBC_IMPL_TIME_SECS_PER_MIN);
    tm->tm_min = (int) (secs / _LIBC_IMPL_TIME_SECS_PER_MIN % _LIBC_IMPL_TIME_MINS_PER_HOUR);
    tm->tm_hour = (int) (secs / _LIBC_IMPL_TIME_SECS_PER_HOUR);
    tm->tm_mday = (int) (doy - _Libc_Impl_Time_MarchDays[march] + _LIBC_IMPL_TIME_FIRST_MDAY);
    tm->tm_mon = mon;
    tm->tm_year = (int) (year - _LIBC_IMPL_TIME_YEAR_BASE);
    tm->tm_wday = (int) _Libc_Impl_Time_FloorMod(days + _LIBC_IMPL_TIME_EPOCH_WDAY, _LIBC_IMPL_TIME_DAYS_PER_WEEK);
    tm->tm_yday = (int) (days - _Libc_Impl_Time_MonthDays(year, _LIBC_IMPL_TIME_JANUARY));
    tm->tm_isdst = 0;
    tm->_Libc_tm_gmtoff = 0;
    tm->_Libc_tm_zone = zone;
    return tm;
}

// Append the character ch to text.
void _Libc_Impl_Time_PutChar(struct _Libc_Impl_Time_Text *text, char ch)
{
    if (text->tt_len < text->tt_size) {
        text->tt_str[text->tt_len] = ch;
    }
    text->tt_len++;
}

// Append at most len characters of the string str to text.
void _Libc_Impl_Time_PutString(struct _Libc_Impl_Time_Text *text, const char *str, _Libc_Impl_Stddef_size_t len)
{
    for (_Libc_Impl_Stddef_size_t i = 0; i < len && str[i] != '\0'; i++) {
        _Libc_Impl_Time_PutChar(text, str[i]);
    }
}

// Append value to text in at least width characters padded with pad.
void _Libc_Impl_Time_PutNumber(struct _Libc_Impl_Time_Text *text, long value, int width, char pad)
{
    unsigned long mag = value < 0 ? 0UL - (unsigned long) value : (unsigned long) value;
    char digits[_LIBC_IMPL_TIME_NUMBER_SIZE];
    int len = 0;
    int fill;

    do {
        digits[len] = (char) ('0' + mag % _LIBC_IMPL_TIME_NUMBER_BASE);
        len++;
        mag /= _LIBC_IMPL_TIME_NUMBER_BASE;
    } while (mag != 0);
    fill = width - len - (value < 0 ? 1 : 0);
    for (; fill > 0 && pad == ' '; fill--) {
        _Libc_Impl_Time_PutChar(text, ' ');
    }
    if (value < 0) {
        _Libc_Impl_Time_PutChar(text, '-');
    }
    for (; fill > 0; fill--) {
        _Libc_Impl_Time_PutChar(text, pad);
    }
    while (len > 0) {
        len--;
        _Libc_Impl_Time_PutChar(text, digits[len]);
    }
}

// Append value to text as printf's %.2d does.
void _Libc_Impl_Time_PutTwoDigits(struct _Libc_Impl_Time_Text *text, int value)
{
    long mag = value;

    if (mag < 0) {
        _Libc_Impl_Time_PutChar(text, '-');
        mag = -mag;
    }
    _Libc_Impl_Time_PutNumber(text, mag, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
}

// Return the name of the weekday wday.
const char *_Libc_Impl_Time_WdayName(int wday)
{
    if (wday < 0 || wday >= _LIBC_IMPL_TIME_DAYS_PER_WEEK) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    return _Libc_Impl_Time_WdayNames[wday];
}

// Return the name of the month mon.
const char *_Libc_Impl_Time_MonName(int mon)
{
    if (mon < 0 || mon >= _LIBC_IMPL_TIME_MONTHS_PER_YEAR) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    return _Libc_Impl_Time_MonNames[mon];
}

// Append at most len characters of the name name to text.
void _Libc_Impl_Time_PutName(struct _Libc_Impl_Time_Text *text, const char *name, _Libc_Impl_Stddef_size_t len, const char *unknown)
{
    if (name == _LIBC_IMPL_STDDEF_NULL) {
        _Libc_Impl_Time_PutString(text, unknown, _Libc_Impl_String_strlen(unknown));
    } else {
        _Libc_Impl_Time_PutString(text, name, len);
    }
}

// Count the days from the start of the ISO 8601 year to the day yday.
long _Libc_Impl_Time_IsoDays(long yday, long wday)
{
    return yday - (yday - wday + _LIBC_IMPL_TIME_ISO_WEEK1_WDAY + _LIBC_IMPL_TIME_ISO_SHIFT) % _LIBC_IMPL_TIME_DAYS_PER_WEEK + _LIBC_IMPL_TIME_ISO_WEEK1_WDAY - _LIBC_IMPL_TIME_ISO_START_WDAY;
}

// Find the ISO 8601 year of tm.
long _Libc_Impl_Time_IsoYear(const struct tm *tm, long *days)
{
    long year = tm->tm_year + _LIBC_IMPL_TIME_YEAR_BASE;
    long count = _Libc_Impl_Time_IsoDays(tm->tm_yday, tm->tm_wday);
    long next;

    if (count < 0) {
        year--;
        count = _Libc_Impl_Time_IsoDays(tm->tm_yday + _Libc_Impl_Time_YearDays(year), tm->tm_wday);
    } else {
        next = _Libc_Impl_Time_IsoDays(tm->tm_yday - _Libc_Impl_Time_YearDays(year), tm->tm_wday);
        if (next >= 0) {
            year++;
            count = next;
        }
    }
    *days = count;
    return year;
}

// Return true if strftime knows the conversion conv after the modifier mod.
_Bool _Libc_Impl_Time_IsConversion(char mod, char conv)
{
    const char *known = _LIBC_IMPL_TIME_CONVERSIONS;

    if (mod == 'E') {
        known = _LIBC_IMPL_TIME_CONVERSIONS_E;
    } else if (mod == 'O') {
        known = _LIBC_IMPL_TIME_CONVERSIONS_O;
    }
    return conv != '\0' && _Libc_Impl_String_strchr(known, conv) != _LIBC_IMPL_STDDEF_NULL;
}

// Return the format strftime's conversion conv stands for.
const char *_Libc_Impl_Time_Composite(char conv)
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
            return _LIBC_IMPL_STDDEF_NULL;
        } break;
    }
}

// Append strftime's conversion conv of tm to text.
void _Libc_Impl_Time_PutConversion(struct _Libc_Impl_Time_Text *text, char conv, const struct tm *tm)
{
    long year = tm->tm_year + _LIBC_IMPL_TIME_YEAR_BASE;
    int hour = tm->tm_hour;
    long days;
    long mins;

    switch (conv) {
        case 'a': {
            _Libc_Impl_Time_PutName(text, _Libc_Impl_Time_WdayName(tm->tm_wday), _LIBC_IMPL_TIME_ABBREV_LEN, _LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'A': {
            _Libc_Impl_Time_PutName(text, _Libc_Impl_Time_WdayName(tm->tm_wday), _LIBC_IMPL_STDINT_SIZE_MAX, _LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'b':
        case 'h': {
            _Libc_Impl_Time_PutName(text, _Libc_Impl_Time_MonName(tm->tm_mon), _LIBC_IMPL_TIME_ABBREV_LEN, _LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'B': {
            _Libc_Impl_Time_PutName(text, _Libc_Impl_Time_MonName(tm->tm_mon), _LIBC_IMPL_STDINT_SIZE_MAX, _LIBC_IMPL_TIME_UNKNOWN_NAME);
        } break;
        case 'C': {
            _Libc_Impl_Time_PutNumber(text, _Libc_Impl_Time_FloorDiv(year, _LIBC_IMPL_TIME_YEARS_PER_CENTURY), _LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'd': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_mday, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'e': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_mday, _LIBC_IMPL_TIME_WIDTH_FIELD, ' ');
        } break;
        case 'g': {
            _Libc_Impl_Time_PutNumber(text, _Libc_Impl_Time_FloorMod(_Libc_Impl_Time_IsoYear(tm, &days), _LIBC_IMPL_TIME_YEARS_PER_CENTURY), _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'G': {
            _Libc_Impl_Time_PutNumber(text, _Libc_Impl_Time_IsoYear(tm, &days), _LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'H': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_hour, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'I': {
            if (hour > _LIBC_IMPL_TIME_HOURS_PER_HALF) {
                hour -= _LIBC_IMPL_TIME_HOURS_PER_HALF;
            } else if (hour == 0) {
                hour = _LIBC_IMPL_TIME_HOURS_PER_HALF;
            }
            _Libc_Impl_Time_PutNumber(text, hour, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'j': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_yday + 1L, _LIBC_IMPL_TIME_WIDTH_YDAY, '0');
        } break;
        case 'm': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_mon + 1L, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'M': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_min, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'n': {
            _Libc_Impl_Time_PutChar(text, '\n');
        } break;
        case 'p': {
            _Libc_Impl_Time_PutString(text, hour >= _LIBC_IMPL_TIME_HOURS_PER_HALF ? "PM" : "AM", _LIBC_IMPL_STDINT_SIZE_MAX);
        } break;
        case 'S': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_sec, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 't': {
            _Libc_Impl_Time_PutChar(text, '\t');
        } break;
        case 'u': {
            _Libc_Impl_Time_PutNumber(text, (tm->tm_wday - 1L + _LIBC_IMPL_TIME_DAYS_PER_WEEK) % _LIBC_IMPL_TIME_DAYS_PER_WEEK + 1, _LIBC_IMPL_TIME_WIDTH_WDAY, '0');
        } break;
        case 'U': {
            _Libc_Impl_Time_PutNumber(text, (tm->tm_yday - (long) tm->tm_wday + _LIBC_IMPL_TIME_DAYS_PER_WEEK) / _LIBC_IMPL_TIME_DAYS_PER_WEEK, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'V': {
            _Libc_Impl_Time_IsoYear(tm, &days);
            _Libc_Impl_Time_PutNumber(text, days / _LIBC_IMPL_TIME_DAYS_PER_WEEK + 1, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'w': {
            _Libc_Impl_Time_PutNumber(text, tm->tm_wday, _LIBC_IMPL_TIME_WIDTH_WDAY, '0');
        } break;
        case 'W': {
            _Libc_Impl_Time_PutNumber(text, (tm->tm_yday - (tm->tm_wday - 1L + _LIBC_IMPL_TIME_DAYS_PER_WEEK) % _LIBC_IMPL_TIME_DAYS_PER_WEEK + _LIBC_IMPL_TIME_DAYS_PER_WEEK) / _LIBC_IMPL_TIME_DAYS_PER_WEEK, _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'y': {
            _Libc_Impl_Time_PutNumber(text, _Libc_Impl_Time_FloorMod(year, _LIBC_IMPL_TIME_YEARS_PER_CENTURY), _LIBC_IMPL_TIME_WIDTH_FIELD, '0');
        } break;
        case 'Y': {
            _Libc_Impl_Time_PutNumber(text, year, _LIBC_IMPL_TIME_WIDTH_YEAR, '0');
        } break;
        case 'z': {
            mins = tm->_Libc_tm_gmtoff / _LIBC_IMPL_TIME_SECS_PER_MIN;
            _Libc_Impl_Time_PutChar(text, tm->_Libc_tm_gmtoff < 0 ? '-' : '+');
            if (mins < 0) {
                mins = -mins;
            }
            _Libc_Impl_Time_PutNumber(text, mins / _LIBC_IMPL_TIME_MINS_PER_HOUR * _LIBC_IMPL_TIME_OFFSET_HOUR + mins % _LIBC_IMPL_TIME_MINS_PER_HOUR, _LIBC_IMPL_TIME_WIDTH_OFFSET, '0');
        } break;
        case 'Z': {
            _Libc_Impl_Time_PutString(text, tm->_Libc_tm_zone != _LIBC_IMPL_STDDEF_NULL ? tm->_Libc_tm_zone : _LIBC_IMPL_TIME_ZONE_UTC, _LIBC_IMPL_STDINT_SIZE_MAX);
        } break;
        default: {
            _Libc_Impl_Time_PutChar(text, '%');
        } break;
    }
}

// Append the conversions of tm that format names to text.
void _Libc_Impl_Time_PutFormat(struct _Libc_Impl_Time_Text *text, const char *format, const struct tm *tm)
{
    const char *ptr = format;

    while (*ptr != '\0') {
        const char *start = ptr;
        const char *composite;
        char mod = '\0';

        if (*ptr != '%') {
            _Libc_Impl_Time_PutChar(text, *ptr);
            ptr++;
            continue;
        }
        ptr++;
        if (*ptr == 'E' || *ptr == 'O') {
            mod = *ptr;
            ptr++;
        }
        if (! _Libc_Impl_Time_IsConversion(mod, *ptr)) {
            if (*ptr != '\0') {
                ptr++;
            }
            _Libc_Impl_Time_PutString(text, start, (_Libc_Impl_Stddef_size_t) (ptr - start));
            continue;
        }
        composite = _Libc_Impl_Time_Composite(*ptr);
        if (composite != _LIBC_IMPL_STDDEF_NULL) {
            _Libc_Impl_Time_PutFormat(text, composite, tm);
        } else {
            _Libc_Impl_Time_PutConversion(text, *ptr, tm);
        }
        ptr++;
    }
}
