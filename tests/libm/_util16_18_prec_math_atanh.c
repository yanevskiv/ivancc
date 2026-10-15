// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_atanh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_atanh should give the inverse hyperbolic tangent of x in each
// type, in the current direction for float and double, keep a zero's sign
// raising nothing, give a pole at +-1 with divide-by-zero and ERANGE, and a NaN
// with invalid and EDOM outside [-1, +1].
static void Test_PrecMathAtanh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.f2272ae325a57p-1, 0x1.f2272ae325a57p-1, 0x1.f2272ae325a58p-1, 0x1.f2272ae325a57p-1};
    static const float valuef[MODES] = {0x1.193ea8p-1f, 0x1.193ea6p-1f, 0x1.193ea8p-1f, 0x1.193ea6p-1f};
    volatile double three = 0.75;
    volatile float halff = 0.5f;
    volatile long double fifthl = 0.2L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_atanh(three), value[i]);
        _TEST_FLOAT(__prec_math_atanhf(halff), valuef[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_atanhl(fifthl), 0xc.f991f65fcc25f96p-6L);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_atanh(0.0), 0.0);
    _TEST_FLOAT(__prec_math_atanhf(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_atanhl(-0.0L), -0.0L);
    _TEST_TRUE(_Exact_IsNan(__prec_math_atanh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_atanh(1.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_atanhl(-1.0L), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(__prec_math_atanh(1.5)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(__prec_math_atanhf((float) inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAtanh();
    return _Test_Status();
}
