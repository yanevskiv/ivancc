// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_log10.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_log10 should give the base-10 logarithm in the current direction
// in each type, a power of 10's exponent, +0 of 1 in every direction, -inf with
// divide-by-zero and ERANGE of a zero, and a NaN with invalid and EDOM below
// zero.
static void Test_PrecMathLog10(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.34413509f79ffp-2, 0x1.34413509f79fep-2, 0x1.34413509f79ffp-2, 0x1.34413509f79fep-2};
    static const float valuef[MODES] = {0x1.344136p-2f, 0x1.344134p-2f, 0x1.344136p-2f, 0x1.344134p-2f};
    static const long double valuel[MODES] = {0x9.a209a84fbcff799p-5L, 0x9.a209a84fbcff798p-5L, 0x9.a209a84fbcff799p-5L, 0x9.a209a84fbcff798p-5L};
    volatile double two = 2.0;
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double one = 1.0;
    volatile double thousand = 1000.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_log10(two), value[i]);
        _TEST_FLOAT(__prec_math_log10f(twof), valuef[i]);
        _TEST_FLOAT(__prec_math_log10l(twol), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_log10(one), 0.0);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _TEST_FLOAT(__prec_math_log10(thousand), 3.0);
    _TEST_FLOAT(__prec_math_log10f(1e10f), 10.0f);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_log10(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_log10l(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_log10(-zero), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_log10f(-1.0f)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathLog10();
    return _Test_Status();
}
