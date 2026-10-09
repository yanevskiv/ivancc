/*
 * C header file for the system layer.
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

#ifndef __INTERNAL_SYS_H__
#define __INTERNAL_SYS_H__

// Linux's clocks.
#define __LIBC_SYS_CLOCK_REALTIME           0
#define __LIBC_SYS_CLOCK_PROCESS_CPUTIME_ID 2

// Linux's struct timespec.
struct __libc_timespec {
    long tv_sec;
    long tv_nsec;
};

// System calls
long __libc_syscall(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
long __libc_write(int fd, const void *buf, unsigned long len);
int __libc_getpid(void);
void __libc_exit(int status);
long __libc_kill(int pid, int sig);
long __libc_clock_gettime(int clock, struct __libc_timespec *spec);

#endif // __INTERNAL_SYS_H__
