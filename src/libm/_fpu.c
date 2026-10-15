/*
 * C source file for the floating-point units' controls and flags.
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

// Module header.
#include <_fpu.h>

// The control words a program starts with: every exception masked, to
// nearest, and the x87 at 64 bits.
#define _FPU_DEFAULT_CONTROL 0x037F
#define _FPU_DEFAULT_MXCSR   0x1F80

// The operands _Fpu_RaiseFlags divides, an exception for each quotient.
#define _FPU_ZERO      0.0f
#define _FPU_ONE       1.0f
#define _FPU_THREE     3.0f
#define _FPU_FLOAT_MAX 0x1.fffffep127f
#define _FPU_FLOAT_MIN 0x1p-126f

#ifdef __x86_64__
// Store both units' environments in env, the x87's masks left as they were.
__asm__(
    "  .text\n"
    "  .globl _Fpu_StoreEnv\n"
    "_Fpu_StoreEnv:\n"
    "  fnstenv (%rdi)\n"
    "  fldenv (%rdi)\n"
    "  stmxcsr 28(%rdi)\n"
    "  ret\n"
);

// Load both units' environments from env.
__asm__(
    "  .text\n"
    "  .globl _Fpu_LoadEnv\n"
    "_Fpu_LoadEnv:\n"
    "  fldenv (%rdi)\n"
    "  ldmxcsr 28(%rdi)\n"
    "  ret\n"
);

// Return a divided by b on SSE, raising what the division raises.
__asm__(
    "  .text\n"
    "  .globl _Fpu_DivideSingle\n"
    "_Fpu_DivideSingle:\n"
    "  divss %xmm1, %xmm0\n"
    "  ret\n"
);
#else
#error "_fpu.c: no floating-point environment for this architecture"
#endif

// Clear the flags of flags in both units.
void _Fpu_ClearFlags(int flags)
{
    _Fpu_Env env;

    _Fpu_StoreEnv(&env);
    env.fe_status &= (unsigned short) ~(flags & _FPU_EXCEPTS);
    env.fe_mxcsr &= (unsigned int) ~(flags & _FPU_EXCEPTS);
    _Fpu_LoadEnv(&env);
}

// Raise each exception of flags by a division that raises it.
void _Fpu_RaiseFlags(int flags)
{
    if (flags & _FPU_INVALID) {
        _Fpu_DivideSingle(_FPU_ZERO, _FPU_ZERO);
    }
    if (flags & _FPU_DIVBYZERO) {
        _Fpu_DivideSingle(_FPU_ONE, _FPU_ZERO);
    }
    if (flags & _FPU_OVERFLOW) {
        _Fpu_DivideSingle(_FPU_FLOAT_MAX, _FPU_FLOAT_MIN);
    }
    if (flags & _FPU_UNDERFLOW) {
        _Fpu_DivideSingle(_FPU_FLOAT_MIN, _FPU_FLOAT_MAX);
    }
    if (flags & _FPU_INEXACT) {
        _Fpu_DivideSingle(_FPU_ONE, _FPU_THREE);
    }
}

// Write the flags of mask in both units as flags holds them.
void _Fpu_WriteFlags(int flags, int mask)
{
    _Fpu_Env env;

    mask &= _FPU_EXCEPTS;
    _Fpu_StoreEnv(&env);
    env.fe_status = (unsigned short) ((env.fe_status & ~mask) | (flags & mask));
    env.fe_mxcsr = (env.fe_mxcsr & (unsigned int) ~mask) | (unsigned int) (flags & mask);
    _Fpu_LoadEnv(&env);
}

// Return which flags of flags either unit has set.
int _Fpu_TestFlags(int flags)
{
    _Fpu_Env env;

    _Fpu_StoreEnv(&env);
    return (int) ((env.fe_status | env.fe_mxcsr) & (unsigned int) (flags & _FPU_EXCEPTS));
}

// Read the rounding direction, the x87's, which _Fpu_WriteRound keeps SSE's.
int _Fpu_ReadRound(void)
{
    _Fpu_Env env;

    _Fpu_StoreEnv(&env);
    return env.fe_control & _FPU_ROUND_MASK;
}

// Write both units' rounding direction, if round names one.
int _Fpu_WriteRound(int round)
{
    _Fpu_Env env;

    if (round & ~_FPU_ROUND_MASK) {
        return _FPU_FAILED;
    }
    _Fpu_StoreEnv(&env);
    env.fe_control = (unsigned short) ((env.fe_control & ~_FPU_ROUND_MASK) | round);
    env.fe_mxcsr &= (unsigned int) ~(_FPU_ROUND_MASK << _FPU_SSE_ROUND_SHIFT);
    env.fe_mxcsr |= (unsigned int) round << _FPU_SSE_ROUND_SHIFT;
    _Fpu_LoadEnv(&env);
    return 0;
}

// Store the environment in env, then clear the flags and mask every exception.
void _Fpu_HoldEnv(_Fpu_Env *env)
{
    _Fpu_Env held;

    _Fpu_StoreEnv(env);
    held = *env;
    held.fe_control |= _FPU_FLAGS;
    held.fe_status &= (unsigned short) ~_FPU_FLAGS;
    held.fe_mxcsr |= _FPU_FLAGS << _FPU_SSE_MASK_SHIFT;
    held.fe_mxcsr &= (unsigned int) ~_FPU_FLAGS;
    _Fpu_LoadEnv(&held);
}

// Write both units' controls and flags from env, the x87's stack kept.
void _Fpu_WriteEnv(const _Fpu_Env *env)
{
    _Fpu_Env now;

    _Fpu_StoreEnv(&now);
    now.fe_status &= (unsigned short) ~_FPU_FLAGS;
    if (env == _FPU_DEFAULT_ENV) {
        now.fe_control = _FPU_DEFAULT_CONTROL;
        now.fe_mxcsr = _FPU_DEFAULT_MXCSR;
    } else {
        now.fe_control = env->fe_control;
        now.fe_status |= env->fe_status & _FPU_FLAGS;
        now.fe_mxcsr = env->fe_mxcsr;
    }
    _Fpu_LoadEnv(&now);
}
