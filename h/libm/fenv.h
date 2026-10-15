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

// The floating-point units' controls and flags.
#include <_fpu.h>

// Floating-point environment
// [C99:7.6p5]: the exceptions, distinct bits
// [C99:7.6p6]: FE_ALL_EXCEPT, their bitwise OR
// [C99:7.6p7]: the rounding directions, distinct nonnegative values
// [C99:7.6p8]: FE_DFL_ENV, the environment at program startup
#define FE_DIVBYZERO  _FPU_DIVBYZERO
#define FE_INEXACT    _FPU_INEXACT
#define FE_INVALID    _FPU_INVALID
#define FE_OVERFLOW   _FPU_OVERFLOW
#define FE_UNDERFLOW  _FPU_UNDERFLOW
#define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)
#define FE_DOWNWARD   _FPU_DOWNWARD
#define FE_TONEAREST  _FPU_TONEAREST
#define FE_TOWARDZERO _FPU_TOWARDZERO
#define FE_UPWARD     _FPU_UPWARD
#define FE_DFL_ENV    _FPU_DEFAULT_ENV

// Floating-point environment
// [C99:7.6p4]: fexcept_t, the status flags
typedef unsigned short fexcept_t;

// Floating-point environment
// [C99:7.6p3]: fenv_t, the whole environment
typedef _Fpu_Env fenv_t;

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
