/*
 * C source file for the floating-point environment.
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
#include <fenv.h>

// Includes.
#include <float.h>

#ifdef __x86_64__
// Store both units' environments in envp, the x87's masks left as they were.
__asm__(
    "  .text\n"
    "  .globl _Fenv_StoreEnv\n"
    "_Fenv_StoreEnv:\n"
    "  fnstenv (%rdi)\n"
    "  fldenv (%rdi)\n"
    "  stmxcsr 28(%rdi)\n"
    "  ret\n"
);

// Load both units' environments from envp.
__asm__(
    "  .text\n"
    "  .globl _Fenv_LoadEnv\n"
    "_Fenv_LoadEnv:\n"
    "  fldenv (%rdi)\n"
    "  ldmxcsr 28(%rdi)\n"
    "  ret\n"
);

// Return a divided by b on SSE, raising what the division raises.
__asm__(
    "  .text\n"
    "  .globl _Fenv_DivideSingle\n"
    "_Fenv_DivideSingle:\n"
    "  divss %xmm1, %xmm0\n"
    "  ret\n"
);
#else
#error "fenv.c: no floating-point environment for this architecture"
#endif

// Clear the flags of excepts in both units.
int feclearexcept(int excepts)
{
    fenv_t env;

    _Fenv_StoreEnv(&env);
    env.fe_status &= (unsigned short) ~(excepts & FE_ALL_EXCEPT);
    env.fe_mxcsr &= (unsigned int) ~(excepts & FE_ALL_EXCEPT);
    _Fenv_LoadEnv(&env);
    return 0;
}

// Store in flagp which flags of excepts either unit has set.
int fegetexceptflag(fexcept_t *flagp, int excepts)
{
    *flagp = (fexcept_t) fetestexcept(excepts);
    return 0;
}

// Raise each exception of excepts by a division that raises it.
int feraiseexcept(int excepts)
{
    if (excepts & FE_INVALID) {
        _Fenv_DivideSingle(_FENV_ZERO, _FENV_ZERO);
    }
    if (excepts & FE_DIVBYZERO) {
        _Fenv_DivideSingle(_FENV_ONE, _FENV_ZERO);
    }
    if (excepts & FE_OVERFLOW) {
        _Fenv_DivideSingle(FLT_MAX, FLT_MIN);
    }
    if (excepts & FE_UNDERFLOW) {
        _Fenv_DivideSingle(FLT_MIN, FLT_MAX);
    }
    if (excepts & FE_INEXACT) {
        _Fenv_DivideSingle(_FENV_ONE, _FENV_THREE);
    }
    return 0;
}

// Set the flags of excepts in both units as flagp holds them.
int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
    fenv_t env;
    int mask = excepts & FE_ALL_EXCEPT;

    _Fenv_StoreEnv(&env);
    env.fe_status = (unsigned short) ((env.fe_status & ~mask) | (*flagp & mask));
    env.fe_mxcsr = (env.fe_mxcsr & (unsigned int) ~mask) | (unsigned int) (*flagp & mask);
    _Fenv_LoadEnv(&env);
    return 0;
}

// Return which flags of excepts either unit has set.
int fetestexcept(int excepts)
{
    fenv_t env;

    _Fenv_StoreEnv(&env);
    return (int) ((env.fe_status | env.fe_mxcsr) & (unsigned int) (excepts & FE_ALL_EXCEPT));
}

// Return the rounding direction, the x87's, which fesetround keeps SSE's.
int fegetround(void)
{
    fenv_t env;

    _Fenv_StoreEnv(&env);
    return env.fe_control & _FENV_ROUND_MASK;
}

// Set both units' rounding direction to round, if it names one.
int fesetround(int round)
{
    fenv_t env;

    if (round & ~_FENV_ROUND_MASK) {
        return _FENV_FAILED;
    }
    _Fenv_StoreEnv(&env);
    env.fe_control = (unsigned short) ((env.fe_control & ~_FENV_ROUND_MASK) | round);
    env.fe_mxcsr &= (unsigned int) ~(_FENV_ROUND_MASK << _FENV_SSE_ROUND_SHIFT);
    env.fe_mxcsr |= (unsigned int) round << _FENV_SSE_ROUND_SHIFT;
    _Fenv_LoadEnv(&env);
    return 0;
}

// Store both units' environments in envp.
int fegetenv(fenv_t *envp)
{
    _Fenv_StoreEnv(envp);
    return 0;
}

// Store the environment in envp, then clear the flags and mask every exception.
int feholdexcept(fenv_t *envp)
{
    fenv_t env;

    _Fenv_StoreEnv(envp);
    env = *envp;
    env.fe_control |= _FENV_FLAGS;
    env.fe_status &= (unsigned short) ~_FENV_FLAGS;
    env.fe_mxcsr |= _FENV_FLAGS << _FENV_SSE_MASK_SHIFT;
    env.fe_mxcsr &= (unsigned int) ~_FENV_FLAGS;
    _Fenv_LoadEnv(&env);
    return 0;
}

// Set both units' controls and flags from envp, the x87's stack kept.
int fesetenv(const fenv_t *envp)
{
    fenv_t env;

    _Fenv_StoreEnv(&env);
    env.fe_status &= (unsigned short) ~_FENV_FLAGS;
    if (envp == FE_DFL_ENV) {
        env.fe_control = _FENV_DEFAULT_CONTROL;
        env.fe_mxcsr = _FENV_DEFAULT_MXCSR;
    } else {
        env.fe_control = envp->fe_control;
        env.fe_status |= envp->fe_status & _FENV_FLAGS;
        env.fe_mxcsr = envp->fe_mxcsr;
    }
    _Fenv_LoadEnv(&env);
    return 0;
}

// Set the environment from envp, then raise the exceptions raised before.
int feupdateenv(const fenv_t *envp)
{
    int raised = fetestexcept(FE_ALL_EXCEPT);

    fesetenv(envp);
    return feraiseexcept(raised);
}
