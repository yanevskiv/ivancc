// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_tgamma.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// __prec_math_tgamma should give gamma of x in each type, a factorial exactly,
// +-inf with divide-by-zero of a zero, a domain error of a negative integer and
// of -inf, and a range error when its result overflows or underflows to zero.
static void Test_PrecMathTgamma(void)
{
    volatile double half = 0.5;
    volatile float halff = 0.5f;
    volatile long double halfl = 0.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    _TEST_FLOAT(__prec_math_tgamma(half), 0x1.c5bf891b4ef6bp+0);
    _TEST_FLOAT(__prec_math_tgammaf(halff), 0x1.c5bf8ap+0f);
    _TEST_FLOAT(__prec_math_tgammal(halfl), 0xe.2dfc48da77b553dp-3L);
    _TEST_FLOAT(__prec_math_tgamma(-2.5), -0x1.e3ff812e32183p-1);
    _TEST_FLOAT(__prec_math_tgammaf(-2.5f), -0x1.e3ff82p-1f);
    _TEST_FLOAT(__prec_math_tgammal(-2.5L), -0xf.1ffc097190c16a7p-4L);
    _TEST_FLOAT(__prec_math_tgamma(30.5), 0x1.305adf049c818p+105);
    _TEST_FLOAT(__prec_math_tgammal(30.5L), 0x9.82d6f824e40bfa2p+102L);
    _TEST_FLOAT(__prec_math_tgamma(5.0), 24.0);
    _TEST_FLOAT(__prec_math_tgammaf(14.0f), 6227020800.0f);
    _TEST_FLOAT(__prec_math_tgammal(27.0L), 403291461126605635584000000.0L);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_tgamma(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_tgamma(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_tgamma(zero), _EXACT_INFINITY);
    _TEST_FLOAT(__prec_math_tgammaf(-0.0f), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_tgamma(-1.0)));
    _TEST_TRUE(_Exact_IsNan(__prec_math_tgammal(-inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_tgamma(172.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_tgamma(-200.5), -0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathTgamma();
    return _Test_Status();
}
