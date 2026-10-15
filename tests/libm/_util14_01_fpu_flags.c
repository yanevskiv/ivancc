// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_fpu.h> flags and rounding.

// Includes.
#include <_test.h>
#include <_fpu.h>

// Raising, writing and clearing flags should set exactly those, and the
// rounding direction should read back as written, a bad one refused.
static void Test_FpuFlags(void)
{
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _Fpu_RaiseFlags(_FPU_INVALID | _FPU_INEXACT);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_INVALID);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_WriteFlags(_FPU_OVERFLOW, _FPU_OVERFLOW | _FPU_INEXACT);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW);
    _Fpu_ClearFlags(_FPU_EXCEPTS);

    _TEST_INT(_Fpu_WriteRound(_FPU_UPWARD), 0);
    _TEST_INT(_Fpu_ReadRound(), _FPU_UPWARD);
    _TEST_INT(_Fpu_WriteRound(_FPU_ROUND_MASK + 1), _FPU_FAILED);
    _TEST_INT(_Fpu_ReadRound(), _FPU_UPWARD);
    _TEST_INT(_Fpu_WriteRound(_FPU_TONEAREST), 0);
    _TEST_INT(_Fpu_ReadRound(), _FPU_TONEAREST);
}

int main(void)
{
    Test_FpuFlags();
    return _Test_Status();
}
