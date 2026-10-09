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
#define _SYS_NR_WRITE         1
#define _SYS_NR_BRK           12
#define _SYS_NR_RT_SIGACTION  13
#define _SYS_NR_GETPID        39
#define _SYS_NR_EXIT          60
#define _SYS_NR_KILL          62
#define _SYS_NR_CLOCK_GETTIME 228

// Linux's rt_sigreturn number, as the restorer's assembly writes it.
#define _SYS_NR_RT_SIGRETURN "15"

// The bytes of Linux's sigset_t.
#define _SYS_SIGSET_SIZE 8

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

// Write len bytes of buf to the descriptor fd.
long _Sys_Write(int fd, const void *buf, unsigned long len)
{
    return _Sys_Syscall(_SYS_NR_WRITE, fd, (long) buf, (long) len, 0, 0, 0);
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

// Return the program's process ID.
int _Sys_Getpid(void)
{
    return (int) _Sys_Syscall(_SYS_NR_GETPID, 0, 0, 0, 0, 0, 0);
}

// End the program with a status.
void _Sys_Exit(int status)
{
    _Sys_Syscall(_SYS_NR_EXIT, status, 0, 0, 0, 0, 0);
}

// Send the signal sig to the process pid.
long _Sys_Kill(int pid, int sig)
{
    return _Sys_Syscall(_SYS_NR_KILL, pid, sig, 0, 0, 0, 0);
}

// Read the clock clock into spec.
long _Sys_ClockGettime(int clock, struct _Sys_Timespec *spec)
{
    return _Sys_Syscall(_SYS_NR_CLOCK_GETTIME, clock, (long) spec, 0, 0, 0, 0);
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
