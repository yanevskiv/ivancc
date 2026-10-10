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

#ifndef _SYS_H
#define _SYS_H

// Linux's clocks.
#define _SYS_CLOCK_REALTIME           0
#define _SYS_CLOCK_PROCESS_CPUTIME_ID 2

// Linux's error numbers C99 does not name.
#define _SYS_ENOMEM    12
#define _SYS_EINVAL    22
#define _SYS_ENOSYS    38
#define _SYS_EOVERFLOW 75

// Linux's first and last signals.
#define _SYS_SIGNAL_FIRST 1
#define _SYS_SIGNAL_LAST  64

// Linux's sigaction flags.
#define _SYS_SA_RESTORER 0x04000000
#define _SYS_SA_RESTART  0x10000000

// Linux's rt_sigprocmask requests.
#define _SYS_SIG_UNBLOCK 1

// Linux's struct timespec.
struct _Sys_Timespec {
    long tv_sec;
    long tv_nsec;
};

// Linux's struct sigaction.
struct _Sys_Sigaction {
    void (*sa_handler)(int);
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    unsigned long sa_mask;
};

// System calls
long _Sys_Syscall(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
long _Sys_Write(int fd, const void *buf, unsigned long len);
void *_Sys_Brk(void *addr);
long _Sys_RtSigaction(int sig, const struct _Sys_Sigaction *act, struct _Sys_Sigaction *oact);
long _Sys_RtSigprocmask(int how, const unsigned long *set, unsigned long *oset);
int _Sys_Getpid(void);
long _Sys_Kill(int pid, int sig);
long _Sys_ClockGettime(int clock, struct _Sys_Timespec *spec);
void _Sys_ExitGroup(int status);

// Signal handlers
void _Sys_RestoreRt(void);

#endif // _SYS_H
