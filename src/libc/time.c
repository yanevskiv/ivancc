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

// The implementation.
#include <libc/impl/libc_time.h>

// Return the processor time the program has used.
clock_t clock(void)
{
    return __libc_impl_time_clock();
}

// Return the seconds from time0 to time1.
double difftime(time_t time1, time_t time0)
{
    return __libc_impl_time_difftime(time1, time0);
}

// Convert the local time timeptr to a calendar time.
time_t mktime(struct tm *timeptr)
{
    return __libc_impl_time_mktime(timeptr);
}

// Return the current calendar time.
time_t time(time_t *timer)
{
    return __libc_impl_time_time(timer);
}

// Write the broken-down time timeptr as text.
char *asctime(const struct tm *timeptr)
{
    return __libc_impl_time_asctime(timeptr);
}

// Write the local time of timer as text.
char *ctime(const time_t *timer)
{
    return __libc_impl_time_ctime(timer);
}

// Break the time timer down as a time of UTC.
struct tm *gmtime(const time_t *timer)
{
    return __libc_impl_time_gmtime(timer);
}

// Break the time timer down as a local time.
struct tm *localtime(const time_t *timer)
{
    return __libc_impl_time_localtime(timer);
}

// Write the conversions of timeptr that format names into str.
size_t strftime(char *restrict str, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr)
{
    return __libc_impl_time_strftime(str, maxsize, format, timeptr);
}
