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

#ifndef __TIME_H__
#define __TIME_H__

// The implementation, which defines struct tm.
#include <libc/impl/libc_time.h>

// (S7.23.1) Components of time
#ifndef __SIZE_T__
#define __SIZE_T__
typedef __libc_impl_stddef_size_t size_t;
#endif

#define NULL __LIBC_IMPL_STDDEF_NULL
#define CLOCKS_PER_SEC __LIBC_IMPL_TIME_CLOCKS_PER_SEC

typedef __libc_impl_time_clock_t clock_t;
typedef __libc_impl_time_time_t time_t;

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

#endif // __TIME_H__
