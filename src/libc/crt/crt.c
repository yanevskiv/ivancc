/*
 * C source file for the program startup.
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
#include <crt/crt.h>

// The system call the startup exits with.
#include <ivancc/libc_sys.h>

// The ivanemu machine's halt register.
#define __CRT_IVANEMU_HALT "0x10000008"

// The program's name for diagnostics, or a null pointer where it is unknown.
char *__crt_argv0;

#ifdef __x86_64__
#if defined(__linux__)
// Keep the program's name, run main, and exit with its status.
__asm__(
    "  .text\n"
    "  .globl _start\n"
    "_start:\n"
    "  xor %rbp, %rbp\n"
    "  mov (%rsp), %rdi\n"
    "  lea 8(%rsp), %rsi\n"
    "  mov (%rsi), %rax\n"
    "  lea __crt_argv0(%rip), %rcx\n"
    "  mov %rax, (%rcx)\n"
    "  mov %rdi, %rdx\n"
    "  mov $3, %rcx\n"
    "  shl %cl, %rdx\n"
    "  add %rsi, %rdx\n"
    "  add $8, %rdx\n"
    "  call main\n"
    "  mov %rax, %rdi\n"
    "  call __libc_sys_exit\n"
);
#elif defined(__ivanemu__)
// Call main, then halt the machine with its status.
__asm__(
    "  .text\n"
    "  .globl _start\n"
    "_start:\n"
    "  call main\n"
    "  mov $" __CRT_IVANEMU_HALT ", %rdi\n"
    "  mov %al, (%rdi)\n"
    ".Lhang:\n"
    "  jmp .Lhang\n"
);
#else
#error "crt.c: no startup for this target"
#endif
#else
#error "crt.c: no startup for this architecture"
#endif
