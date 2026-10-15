// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_erf.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_erf should give the error function of x in the current direction
// in each type, keep a zero's sign, give +-1 of an infinity raising nothing,
// and give a subnormal x's value with underflow.
static void Test_PrecMathErf(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.6c1c9759d0e5fp-1, 0x1.6c1c9759d0e5fp-1, 0x1.6c1c9759d0e6p-1, 0x1.6c1c9759d0e5fp-1};
    static const float valuef[MODES] = {0x1.6c1c98p-1f, 0x1.6c1c96p-1f, 0x1.6c1c98p-1f, 0x1.6c1c96p-1f};
    static const long double valuel[MODES] = {0xb.60e4bace872fb63p-4L, 0xb.60e4bace872fb62p-4L, 0xb.60e4bace872fb63p-4L, 0xb.60e4bace872fb62p-4L};
    volatile double three = 0.75;
    volatile float threef = 0.75f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_erf(three), value[i]);
        _TEST_FLOAT(__prec_math_erff(threef), valuef[i]);
        _TEST_FLOAT(__prec_math_erfl(threel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_erf(0.0), 0.0);
    _TEST_FLOAT(__prec_math_erff(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_erfl(-inf), -1.0L);
    _TEST_FLOAT(__prec_math_erf(inf), 1.0);
    _TEST_TRUE(_Exact_IsNan(__prec_math_erf(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_erf(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_erf(10.5), 1.0);
    _TEST_FLOAT(__prec_math_erfl(-10.5L), -1.0L);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathErf();
    return _Test_Status();
}
