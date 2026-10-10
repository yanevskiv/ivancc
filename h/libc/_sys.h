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

// The descriptors of a program's standard input, output and error.
#define _SYS_STDIN_FILENO  0
#define _SYS_STDOUT_FILENO 1
#define _SYS_STDERR_FILENO 2

// Linux's ioctl that reads a terminal's settings, and its control characters.
#define _SYS_TCGETS 0x5401
#define _SYS_NCCS   19

// Linux's clocks.
#define _SYS_CLOCK_REALTIME           0
#define _SYS_CLOCK_PROCESS_CPUTIME_ID 2

// Linux's error numbers C99 does not name.
#define _SYS_EPERM           1
#define _SYS_ENOENT          2
#define _SYS_ESRCH           3
#define _SYS_EINTR           4
#define _SYS_EIO             5
#define _SYS_ENXIO           6
#define _SYS_E2BIG           7
#define _SYS_ENOEXEC         8
#define _SYS_EBADF           9
#define _SYS_ECHILD          10
#define _SYS_EAGAIN          11
#define _SYS_ENOMEM          12
#define _SYS_EACCES          13
#define _SYS_EFAULT          14
#define _SYS_ENOTBLK         15
#define _SYS_EBUSY           16
#define _SYS_EEXIST          17
#define _SYS_EXDEV           18
#define _SYS_ENODEV          19
#define _SYS_ENOTDIR         20
#define _SYS_EISDIR          21
#define _SYS_EINVAL          22
#define _SYS_ENFILE          23
#define _SYS_EMFILE          24
#define _SYS_ENOTTY          25
#define _SYS_ETXTBSY         26
#define _SYS_EFBIG           27
#define _SYS_ENOSPC          28
#define _SYS_ESPIPE          29
#define _SYS_EROFS           30
#define _SYS_EMLINK          31
#define _SYS_EPIPE           32
#define _SYS_EDEADLK         35
#define _SYS_ENAMETOOLONG    36
#define _SYS_ENOLCK          37
#define _SYS_ENOSYS          38
#define _SYS_ENOTEMPTY       39
#define _SYS_ELOOP           40
#define _SYS_ENOMSG          42
#define _SYS_EIDRM           43
#define _SYS_ECHRNG          44
#define _SYS_EL2NSYNC        45
#define _SYS_EL3HLT          46
#define _SYS_EL3RST          47
#define _SYS_ELNRNG          48
#define _SYS_EUNATCH         49
#define _SYS_ENOCSI          50
#define _SYS_EL2HLT          51
#define _SYS_EBADE           52
#define _SYS_EBADR           53
#define _SYS_EXFULL          54
#define _SYS_ENOANO          55
#define _SYS_EBADRQC         56
#define _SYS_EBADSLT         57
#define _SYS_EBFONT          59
#define _SYS_ENOSTR          60
#define _SYS_ENODATA         61
#define _SYS_ETIME           62
#define _SYS_ENOSR           63
#define _SYS_ENONET          64
#define _SYS_ENOPKG          65
#define _SYS_EREMOTE         66
#define _SYS_ENOLINK         67
#define _SYS_EADV            68
#define _SYS_ESRMNT          69
#define _SYS_ECOMM           70
#define _SYS_EPROTO          71
#define _SYS_EMULTIHOP       72
#define _SYS_EDOTDOT         73
#define _SYS_EBADMSG         74
#define _SYS_EOVERFLOW       75
#define _SYS_ENOTUNIQ        76
#define _SYS_EBADFD          77
#define _SYS_EREMCHG         78
#define _SYS_ELIBACC         79
#define _SYS_ELIBBAD         80
#define _SYS_ELIBSCN         81
#define _SYS_ELIBMAX         82
#define _SYS_ELIBEXEC        83
#define _SYS_ERESTART        85
#define _SYS_ESTRPIPE        86
#define _SYS_EUSERS          87
#define _SYS_ENOTSOCK        88
#define _SYS_EDESTADDRREQ    89
#define _SYS_EMSGSIZE        90
#define _SYS_EPROTOTYPE      91
#define _SYS_ENOPROTOOPT     92
#define _SYS_EPROTONOSUPPORT 93
#define _SYS_ESOCKTNOSUPPORT 94
#define _SYS_EOPNOTSUPP      95
#define _SYS_EPFNOSUPPORT    96
#define _SYS_EAFNOSUPPORT    97
#define _SYS_EADDRINUSE      98
#define _SYS_EADDRNOTAVAIL   99
#define _SYS_ENETDOWN        100
#define _SYS_ENETUNREACH     101
#define _SYS_ENETRESET       102
#define _SYS_ECONNABORTED    103
#define _SYS_ECONNRESET      104
#define _SYS_ENOBUFS         105
#define _SYS_EISCONN         106
#define _SYS_ENOTCONN        107
#define _SYS_ESHUTDOWN       108
#define _SYS_ETOOMANYREFS    109
#define _SYS_ETIMEDOUT       110
#define _SYS_ECONNREFUSED    111
#define _SYS_EHOSTDOWN       112
#define _SYS_EHOSTUNREACH    113
#define _SYS_EALREADY        114
#define _SYS_EINPROGRESS     115
#define _SYS_ESTALE          116
#define _SYS_EUCLEAN         117
#define _SYS_ENOTNAM         118
#define _SYS_ENAVAIL         119
#define _SYS_EISNAM          120
#define _SYS_EREMOTEIO       121
#define _SYS_EDQUOT          122
#define _SYS_ENOMEDIUM       123
#define _SYS_EMEDIUMTYPE     124
#define _SYS_ECANCELED       125
#define _SYS_ENOKEY          126
#define _SYS_EKEYEXPIRED     127
#define _SYS_EKEYREVOKED     128
#define _SYS_EKEYREJECTED    129
#define _SYS_EOWNERDEAD      130
#define _SYS_ENOTRECOVERABLE 131
#define _SYS_ERFKILL         132
#define _SYS_EHWPOISON       133

// Linux's first and last signals.
#define _SYS_SIGNAL_FIRST 1
#define _SYS_SIGNAL_LAST  64

// Linux's sigaction flags.
#define _SYS_SA_RESTORER 0x04000000
#define _SYS_SA_RESTART  0x10000000

// Linux's rt_sigprocmask requests.
#define _SYS_SIG_UNBLOCK 1

// Linux's struct termios, the kernel's and not glibc's.
struct _Sys_Termios {
    unsigned int c_iflag;
    unsigned int c_oflag;
    unsigned int c_cflag;
    unsigned int c_lflag;
    unsigned char c_line;
    unsigned char c_cc[_SYS_NCCS];
};

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
long  _Sys_Syscall(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
long  _Sys_Write(int fd, const void *buf, unsigned long len);
void *_Sys_Brk(void *addr);
long  _Sys_RtSigaction(int sig, const struct _Sys_Sigaction *act, struct _Sys_Sigaction *oact);
long  _Sys_RtSigprocmask(int how, const unsigned long *set, unsigned long *oset);
long  _Sys_Ioctl(int fd, unsigned long request, void *arg);
int   _Sys_Getpid(void);
long  _Sys_Kill(int pid, int sig);
long  _Sys_ClockGettime(int clock, struct _Sys_Timespec *spec);
void  _Sys_ExitGroup(int status);

// Signal handlers
void _Sys_RestoreRt(void);

#endif // _SYS_H
