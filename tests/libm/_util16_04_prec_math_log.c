// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_log.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_log should give the natural logarithm in the current direction in
// each type, +0 of 1 in every direction, -inf with divide-by-zero and ERANGE of
// a zero, and a NaN with invalid and EDOM below zero.
static void Test_PrecMathLog(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.62e42fefa39efp-1, 0x1.62e42fefa39efp-1, 0x1.62e42fefa39fp-1, 0x1.62e42fefa39efp-1};
    static const float valuef[MODES] = {0x1.62e43p-1f, 0x1.62e42ep-1f, 0x1.62e43p-1f, 0x1.62e42ep-1f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double two = 2.0;
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double one = 1.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_log(two), value[i]);
        _TEST_FLOAT(__prec_math_logf(twof), valuef[i]);
        _TEST_FLOAT(__prec_math_logl(twol), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_log(one), 0.0);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _TEST_FLOAT(__prec_math_log(0x1p-1074), -0x1.74385446d71c3p+9);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_logl(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_log(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_log(zero), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_logf(-0.0f), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_log(-one)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_logl(-inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathLog();
    return _Test_Status();
}
