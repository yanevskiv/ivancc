// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> exact logarithms and powers of 2.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// An exact logarithm or power of 2 should be exact in every direction and
// raise nothing, the others within 2 ulps and mostly correctly rounded:
// expm1 of 1 to nearest, expm1l of -1000 inexact, and log1p of the least
// subnormal 0 downward.
static void Test_PrecMathExactLogarithms(void)
{
    volatile double thousand = 1000.0;
    volatile double eight = 8.0;
    volatile double power = -1074.0;
    volatile double one = 1.0;
    volatile long double minus = -1000.0L;
    volatile double least = 0x1p-1074;

    _Fpu_WriteRound(_FPU_DOWNWARD);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_log10(thousand), 3.0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _Fpu_WriteRound(_FPU_UPWARD);
    _TEST_FLOAT(__prec_math_log2(eight), 3.0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_exp2(power), 0x1p-1074);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _TEST_FLOAT(__prec_math_expm1(one), 0x1.b7e151628aed3p+0);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    __prec_math_expm1l(minus);
    _TEST_INT(_Fpu_TestFlags(_FPU_INEXACT), _FPU_INEXACT);
    _Fpu_WriteRound(_FPU_DOWNWARD);
    _TEST_FLOAT(__prec_math_log1p(least), 0.0);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathExactLogarithms();
    return _Test_Status();
}
