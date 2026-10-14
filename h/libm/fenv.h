/*
 * C header file for the floating-point environment.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _FENV_H
#define _FENV_H

// What a function returns when it does not do what it was asked.
#define _FENV_FAILED 1

// The exception flags, the same six bits in the x87 status word and MXCSR.
#define _FENV_FLAGS 0x3F

// The rounding control of the x87 control word.
#define _FENV_ROUND_MASK 0x0C00

// How far above the x87 control word's fields MXCSR holds the same ones.
#define _FENV_SSE_ROUND_SHIFT 3
#define _FENV_SSE_MASK_SHIFT  7

// The control words a program starts with: every exception masked, to
// nearest, and the x87 at 64 bits.
#define _FENV_DEFAULT_CONTROL 0x037F
#define _FENV_DEFAULT_MXCSR   0x1F80

// The dwords of the x87 environment that hold its last pointers and opcode.
#define _FENV_X87_POINTERS 4

// The operands feraiseexcept divides, an exception for each quotient.
#define _FENV_ZERO  0.0f
#define _FENV_ONE   1.0f
#define _FENV_THREE 3.0f

// Floating-point environment
// [C99:7.6p5]: the exceptions, distinct bits
// [C99:7.6p6]: FE_ALL_EXCEPT, their bitwise OR
// [C99:7.6p7]: the rounding directions, distinct nonnegative values
// [C99:7.6p8]: FE_DFL_ENV, the environment at program startup
#define FE_DIVBYZERO  0x04
#define FE_INEXACT    0x20
#define FE_INVALID    0x01
#define FE_OVERFLOW   0x08
#define FE_UNDERFLOW  0x10
#define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)
#define FE_DOWNWARD   0x0400
#define FE_TONEAREST  0x0000
#define FE_TOWARDZERO 0x0C00
#define FE_UPWARD     0x0800
#define FE_DFL_ENV    ((const fenv_t *) -1)

// Floating-point environment
// [C99:7.6p4]: fexcept_t, the status flags
typedef unsigned short fexcept_t;

// Floating-point environment
// [C99:7.6p3]: fenv_t, the whole environment
typedef struct {
    unsigned short fe_control;                    // the x87 control word
    unsigned short fe_unused1;                    // reserved
    unsigned short fe_status;                     // the x87 status word
    unsigned short fe_unused2;                    // reserved
    unsigned short fe_tags;                       // the x87 tag word
    unsigned short fe_unused3;                    // reserved
    unsigned int fe_pointers[_FENV_X87_POINTERS]; // the last opcode and addresses
    unsigned int fe_mxcsr;                        // SSE's control and status
} fenv_t;

// Both units' environments, and a division on SSE
void  _Fenv_StoreEnv(fenv_t *envp);
void  _Fenv_LoadEnv(const fenv_t *envp);
float _Fenv_DivideSingle(float a, float b);

// Floating-point exceptions
// [C99:7.6.2]: feclearexcept to fetestexcept, the five flag functions
int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);

// Rounding
// [C99:7.6.3]: fegetround and fesetround
int fegetround(void);
int fesetround(int round);

// Environment
// [C99:7.6.4]: fegetenv, feholdexcept, fesetenv and feupdateenv
int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

#endif // _FENV_H
