/*
 * C source file for the system layer.
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
#include <_sys.h>

#ifndef __linux__
#error "_sys.c: the system layer is Linux's alone"
#endif

// Linux's syscall numbers.
#define _SYS_NR_READ           0
#define _SYS_NR_WRITE          1
#define _SYS_NR_OPEN           2
#define _SYS_NR_CLOSE          3
#define _SYS_NR_LSTAT          6
#define _SYS_NR_LSEEK          8
#define _SYS_NR_BRK            12
#define _SYS_NR_RT_SIGACTION   13
#define _SYS_NR_RT_SIGPROCMASK 14
#define _SYS_NR_IOCTL          16
#define _SYS_NR_DUP2           33
#define _SYS_NR_GETPID         39
#define _SYS_NR_KILL           62
#define _SYS_NR_RENAME         82
#define _SYS_NR_RMDIR          84
#define _SYS_NR_UNLINK         87
#define _SYS_NR_CLOCK_GETTIME  228
#define _SYS_NR_EXIT_GROUP     231

// The bytes of Linux's struct stat.
#define _SYS_STAT_SIZE 144

// The bytes of Linux's struct termios.
#define _SYS_TERMIOS_SIZE 36

// Linux's rt_sigreturn number, as the restorer's assembly writes it.
#define _SYS_NR_RT_SIGRETURN "15"

// The bytes of Linux's sigset_t.
#define _SYS_SIGSET_SIZE 8

// Check that struct stat is as large as the kernel's.
typedef char _Sys_CheckStat[sizeof(struct _Sys_Stat) == _SYS_STAT_SIZE ? 1 : -1];

// Check that struct termios is laid out as the kernel's.
typedef char _Sys_CheckTermios[sizeof(struct _Sys_Termios) == _SYS_TERMIOS_SIZE ? 1 : -1];

#ifdef __x86_64__
// Make a syscall with its number and six arguments.
__asm__(
    "  .text\n"
    "  .globl _Sys_Syscall\n"
    "_Sys_Syscall:\n"
    "  mov %rdi, %rax\n"
    "  mov %rsi, %rdi\n"
    "  mov %rdx, %rsi\n"
    "  mov %rcx, %rdx\n"
    "  mov %r8, %r10\n"
    "  mov %r9, %r8\n"
    "  mov 8(%rsp), %r9\n"
    "  syscall\n"
    "  ret\n"
);
#else
#error "_sys.c: no system calls for this architecture"
#endif

// Read up to len bytes from the descriptor fd into buf.
long _Sys_Read(int fd, void *buf, unsigned long len)
{
    return _Sys_Syscall(_SYS_NR_READ, fd, (long) buf, (long) len, 0, 0, 0);
}

// Write len bytes of buf to the descriptor fd.
long _Sys_Write(int fd, const void *buf, unsigned long len)
{
    return _Sys_Syscall(_SYS_NR_WRITE, fd, (long) buf, (long) len, 0, 0, 0);
}

// Open the file at path as flags say, with the permissions mode if it is made.
long _Sys_Open(const char *path, int flags, int mode)
{
    return _Sys_Syscall(_SYS_NR_OPEN, (long) path, flags, mode, 0, 0, 0);
}

// Close the descriptor fd.
long _Sys_Close(int fd)
{
    return _Sys_Syscall(_SYS_NR_CLOSE, fd, 0, 0, 0, 0, 0);
}

// Fill info with what the file at path is, a link's own and not its target's.
long _Sys_Lstat(const char *path, struct _Sys_Stat *info)
{
    return _Sys_Syscall(_SYS_NR_LSTAT, (long) path, (long) info, 0, 0, 0, 0);
}

// Move the offset of the descriptor fd by offset from the origin whence.
long _Sys_Lseek(int fd, long offset, int whence)
{
    return _Sys_Syscall(_SYS_NR_LSEEK, fd, offset, whence, 0, 0, 0);
}

// Move the program break to addr, and return where it stands.
void *_Sys_Brk(void *addr)
{
    return (void *) _Sys_Syscall(_SYS_NR_BRK, (long) addr, 0, 0, 0, 0, 0);
}

// Install act as the action of the signal sig, and store the old one in oact.
long _Sys_RtSigaction(int sig, const struct _Sys_Sigaction *act, struct _Sys_Sigaction *oact)
{
    return _Sys_Syscall(_SYS_NR_RT_SIGACTION, sig, (long) act, (long) oact, _SYS_SIGSET_SIZE, 0, 0);
}

// Change the signal mask by how and set, and store the old one in oset.
long _Sys_RtSigprocmask(int how, const unsigned long *set, unsigned long *oset)
{
    return _Sys_Syscall(_SYS_NR_RT_SIGPROCMASK, how, (long) set, (long) oset, _SYS_SIGSET_SIZE, 0, 0);
}

// Make the device request request of the descriptor fd, with its argument arg.
long _Sys_Ioctl(int fd, unsigned long request, void *arg)
{
    return _Sys_Syscall(_SYS_NR_IOCTL, fd, (long) request, (long) arg, 0, 0, 0);
}

// Make the descriptor as a copy of the descriptor fd, closing what it was.
long _Sys_Dup2(int fd, int as)
{
    return _Sys_Syscall(_SYS_NR_DUP2, fd, as, 0, 0, 0, 0);
}

// Return the program's process ID.
int _Sys_Getpid(void)
{
    return (int) _Sys_Syscall(_SYS_NR_GETPID, 0, 0, 0, 0, 0, 0);
}

// Send the signal sig to the process pid.
long _Sys_Kill(int pid, int sig)
{
    return _Sys_Syscall(_SYS_NR_KILL, pid, sig, 0, 0, 0, 0);
}

// Give the file at from the name to, replacing a file that has it.
long _Sys_Rename(const char *from, const char *to)
{
    return _Sys_Syscall(_SYS_NR_RENAME, (long) from, (long) to, 0, 0, 0, 0);
}

// Remove the empty directory at path.
long _Sys_Rmdir(const char *path)
{
    return _Sys_Syscall(_SYS_NR_RMDIR, (long) path, 0, 0, 0, 0, 0);
}

// Remove the name path from its file, which goes with its last name and user.
long _Sys_Unlink(const char *path)
{
    return _Sys_Syscall(_SYS_NR_UNLINK, (long) path, 0, 0, 0, 0, 0);
}

// Read the clock clock into spec.
long _Sys_ClockGettime(int clock, struct _Sys_Timespec *spec)
{
    return _Sys_Syscall(_SYS_NR_CLOCK_GETTIME, clock, (long) spec, 0, 0, 0, 0);
}

// End the program, every thread of it, with a status.
void _Sys_ExitGroup(int status)
{
    _Sys_Syscall(_SYS_NR_EXIT_GROUP, status, 0, 0, 0, 0, 0);
}

#ifdef __x86_64__
// Return from a signal handler to what the signal interrupted.
__asm__(
    "  .text\n"
    "  .globl _Sys_RestoreRt\n"
    "_Sys_RestoreRt:\n"
    "  mov $" _SYS_NR_RT_SIGRETURN ", %rax\n"
    "  syscall\n"
);
#else
#error "_sys.c: no signal handlers for this architecture"
#endif
