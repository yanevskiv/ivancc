/*
 * C source file for nonlocal jumps.
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
#include <setjmp.h>

#ifdef __x86_64__
// Save the callee-saved registers, stack pointer and return address in env.
__asm__(
    "  .text\n"
    "  .globl setjmp\n"
    "setjmp:\n"
    "  mov %rbx, (%rdi)\n"
    "  mov %rbp, 8(%rdi)\n"
    "  mov %r12, 16(%rdi)\n"
    "  mov %r13, 24(%rdi)\n"
    "  mov %r14, 32(%rdi)\n"
    "  mov %r15, 40(%rdi)\n"
    "  lea 8(%rsp), %rdx\n"
    "  mov %rdx, 48(%rdi)\n"
    "  mov (%rsp), %rdx\n"
    "  mov %rdx, 56(%rdi)\n"
    "  xor %rax, %rax\n"
    "  ret\n"
);

// Return val, or 1 for 0, from the setjmp that saved env.
__asm__(
    "  .text\n"
    "  .globl longjmp\n"
    "longjmp:\n"
    "  movslq %esi, %rax\n"
    "  cmp $0, %rax\n"
    "  jne .Lnonzero\n"
    "  mov $1, %rax\n"
    ".Lnonzero:\n"
    "  mov (%rdi), %rbx\n"
    "  mov 8(%rdi), %rbp\n"
    "  mov 16(%rdi), %r12\n"
    "  mov 24(%rdi), %r13\n"
    "  mov 32(%rdi), %r14\n"
    "  mov 40(%rdi), %r15\n"
    "  mov 48(%rdi), %rsp\n"
    "  mov 56(%rdi), %rdx\n"
    "  push %rdx\n"
    "  ret\n"
);
#else
#error "setjmp.c: no nonlocal jumps for this architecture"
#endif
