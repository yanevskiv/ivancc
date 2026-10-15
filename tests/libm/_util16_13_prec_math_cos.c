// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_cos.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_cos should give the cosine of x in radians in each type, in the
// current direction for float and long double, 1 of a zero raising nothing,
// the cosine of a large x, and a NaN with invalid and EDOM of an infinity.
static void Test_PrecMathCos(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.aa2266p-2f, -0x1.aa2266p-2f, -0x1.aa2264p-2f, -0x1.aa2264p-2f};
    static const long double valuel[MODES] = {0xf.80aa4fbef750ba8p-4L, 0xf.80aa4fbef750ba7p-4L, 0xf.80aa4fbef750ba8p-4L, 0xf.80aa4fbef750ba7p-4L};
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double quarterl = 0.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_cosf(twof), valuef[i]);
        _TEST_FLOAT(__prec_math_cosl(quarterl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_cos(half), 0x1.c1528065b7d5p-1);
    _TEST_FLOAT(__prec_math_cos(1e22), 0x1.0be2cef01c8f4p-1);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_cos(0.0), 1.0);
    _TEST_FLOAT(__prec_math_cosf(-0.0f), 1.0f);
    _TEST_FLOAT(__prec_math_cosl(-0.0L), 1.0L);
    _TEST_TRUE(_Exact_IsNan(__prec_math_cos(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_cos(-inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(__prec_math_cosl(inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathCos();
    return _Test_Status();
}
