// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_lgamma.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_lgamma should give the natural logarithm of |gamma| of x in the current
// direction in each type, +0 of 1 and 2 exactly, +inf of the infinities
// raising nothing, +inf with divide-by-zero of a pole, and a range error
// when its result overflows.
static void Test_PrecMathLgamma(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.2383e809a67e8p-2, 0x1.2383e809a67e7p-2, 0x1.2383e809a67e8p-2, 0x1.2383e809a67e7p-2};
    static const float valuef[MODES] = {0x1.2383e8p-2f, 0x1.2383e8p-2f, 0x1.2383eap-2f, 0x1.2383e8p-2f};
    static const long double valuel[MODES] = {0x9.1c1f404d33f3cfcp-5L, 0x9.1c1f404d33f3cfbp-5L, 0x9.1c1f404d33f3cfcp-5L, 0x9.1c1f404d33f3cfbp-5L};
    volatile double five = 2.5;
    volatile float fivef = 2.5f;
    volatile long double fivel = 2.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_lgamma(five), value[i]);
        _TEST_FLOAT(_prec_lgammaf(fivef), valuef[i]);
        _TEST_FLOAT(_prec_lgammal(fivel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _TEST_FLOAT(_prec_lgamma(-0.75), 0x1.93616060ea5ep+0);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_lgamma(1.0), 0.0);
    _TEST_FLOAT(_prec_lgammaf(2.0f), 0.0f);
    _TEST_FLOAT(_prec_lgammal(2.0L), 0.0L);
    _TEST_FLOAT(_prec_lgamma(inf), inf);
    _TEST_FLOAT(_prec_lgammal(-inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_lgamma(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_lgamma(zero), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_lgammal(-3.0L), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_lgamma(0x1.fffffffffffffp+1023), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathLgamma();
    return _Test_Status();
}
