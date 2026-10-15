// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_expm1.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_expm1 should give e to x less 1 in the current direction in each
// type, without cancellation for a small x, keep a zero's sign, give -1 of -inf
// and +inf of +inf, and a range error when it overflows.
static void Test_PrecMathExpm1(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.4c2531c3c0d38p-1, 0x1.4c2531c3c0d37p-1, 0x1.4c2531c3c0d38p-1, 0x1.4c2531c3c0d37p-1};
    static const float valuef[MODES] = {0x1.b7e152p+0f, 0x1.b7e15p+0f, 0x1.b7e152p+0f, 0x1.b7e15p+0f};
    static const long double valuel[MODES] = {-0xa.1d2a7274c4320e5p-4L, -0xa.1d2a7274c4320e6p-4L, -0xa.1d2a7274c4320e5p-4L, -0xa.1d2a7274c4320e5p-4L};
    volatile double half = 0.5;
    volatile float onef = 1.0f;
    volatile long double onel = -1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_expm1(half), value[i]);
        _TEST_FLOAT(__prec_math_expm1f(onef), valuef[i]);
        _TEST_FLOAT(__prec_math_expm1l(onel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_expm1(0.0), 0.0);
    _TEST_FLOAT(__prec_math_expm1f(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_expm1l(-inf), -1.0L);
    _TEST_FLOAT(__prec_math_expm1(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_expm1(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _TEST_FLOAT(__prec_math_expm1(0x1p-60), 0x1p-60);
    _TEST_FLOAT(__prec_math_expm1(-50.0), -1.0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expm1(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_expm1(710.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expm1f(89.0f), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expm1l(11357.0L), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathExpm1();
    return _Test_Status();
}
