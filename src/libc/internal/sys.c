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
#include <internal/sys.h>

#ifndef __linux__
#error "sys.c: the system layer is Linux's alone"
#endif

// Linux's syscall numbers.
#define __LIBC_SYS_NR_WRITE  1
#define __LIBC_SYS_NR_GETPID 39
#define __LIBC_SYS_NR_EXIT   60
#define __LIBC_SYS_NR_KILL   62

#ifdef __x86_64__
// Make a syscall with its number and six arguments.
__asm__(
    "  .text\n"
    "  .globl __libc_syscall\n"
    "__libc_syscall:\n"
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
#error "sys.c: no system calls for this architecture"
#endif

// Write len bytes of buf to the descriptor fd.
long __libc_write(int fd, const void *buf, unsigned long len)
{
    return __libc_syscall(__LIBC_SYS_NR_WRITE, fd, (long) buf, (long) len, 0, 0, 0);
}

// Return the program's process ID.
int __libc_getpid(void)
{
    return (int) __libc_syscall(__LIBC_SYS_NR_GETPID, 0, 0, 0, 0, 0, 0);
}

// End the program with a status.
void __libc_exit(int status)
{
    __libc_syscall(__LIBC_SYS_NR_EXIT, status, 0, 0, 0, 0, 0);
}

// Send the signal sig to the process pid.
long __libc_kill(int pid, int sig)
{
    return __libc_syscall(__LIBC_SYS_NR_KILL, pid, sig, 0, 0, 0, 0);
}
