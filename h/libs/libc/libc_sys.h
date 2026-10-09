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

#ifndef _LIBC_SYS_H
#define _LIBC_SYS_H

// Linux's clocks.
#define _LIBC_SYS_CLOCK_REALTIME           0
#define _LIBC_SYS_CLOCK_PROCESS_CPUTIME_ID 2

// Linux's error numbers C99 does not name.
#define _LIBC_SYS_EINVAL    22
#define _LIBC_SYS_EOVERFLOW 75

// Linux's first and last signals.
#define _LIBC_SYS_SIGNAL_FIRST 1
#define _LIBC_SYS_SIGNAL_LAST  64

// Linux's sigaction flags.
#define _LIBC_SYS_SA_RESTORER 0x04000000
#define _LIBC_SYS_SA_RESTART  0x10000000

// Linux's struct timespec.
struct _Libc_Sys_timespec {
    long tv_sec;
    long tv_nsec;
};

// Linux's struct sigaction.
struct _Libc_Sys_sigaction {
    void (*sa_handler)(int);
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    unsigned long sa_mask;
};

// System calls
long _Libc_Sys_syscall(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
long _Libc_Sys_write(int fd, const void *buf, unsigned long len);
long _Libc_Sys_rt_sigaction(int sig, const struct _Libc_Sys_sigaction *act, struct _Libc_Sys_sigaction *oact);
int _Libc_Sys_getpid(void);
void _Libc_Sys_exit(int status);
long _Libc_Sys_kill(int pid, int sig);
long _Libc_Sys_clock_gettime(int clock, struct _Libc_Sys_timespec *spec);

// Signal handlers
void _Libc_Sys_restore_rt(void);

#endif // _LIBC_SYS_H
