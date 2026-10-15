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

// Clear the flags of excepts in both units.
int feclearexcept(int excepts)
{
    _Fpu_ClearFlags(excepts);
    return 0;
}

// Store in flagp which flags of excepts either unit has set.
int fegetexceptflag(fexcept_t *flagp, int excepts)
{
    *flagp = (fexcept_t) _Fpu_TestFlags(excepts);
    return 0;
}

// Raise each exception of excepts as the arithmetic raising it would.
int feraiseexcept(int excepts)
{
    _Fpu_RaiseFlags(excepts);
    return 0;
}

// Set the flags of excepts in both units as flagp holds them.
int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
    _Fpu_WriteFlags(*flagp, excepts);
    return 0;
}

// Return which flags of excepts either unit has set.
int fetestexcept(int excepts)
{
    return _Fpu_TestFlags(excepts);
}

// Return the rounding direction.
int fegetround(void)
{
    return _Fpu_ReadRound();
}

// Set both units' rounding direction to round, if it names one.
int fesetround(int round)
{
    return _Fpu_WriteRound(round);
}

// Store both units' environments in envp.
int fegetenv(fenv_t *envp)
{
    _Fpu_StoreEnv(envp);
    return 0;
}

// Store the environment in envp, then clear the flags and mask every exception.
int feholdexcept(fenv_t *envp)
{
    _Fpu_HoldEnv(envp);
    return 0;
}

// Set both units' controls and flags from envp, the x87's stack kept.
int fesetenv(const fenv_t *envp)
{
    _Fpu_WriteEnv(envp);
    return 0;
}

// Set the environment from envp, then raise the exceptions raised before.
int feupdateenv(const fenv_t *envp)
{
    int raised = _Fpu_TestFlags(FE_ALL_EXCEPT);

    _Fpu_WriteEnv(envp);
    _Fpu_RaiseFlags(raised);
    return 0;
}
