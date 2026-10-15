// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> exact powers.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// An exact power should be exact in every direction and raise nothing, and
// an overflow rounded to the largest finite value should raise overflow.
static void Test_PrecMathExactPowers(void)
{
    volatile double three = 3.0;
    volatile double nine = 9.0;
    volatile double two = 2.0;
    volatile double thirteen = 13.0;

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Fpu_WriteRound(_FPU_DOWNWARD);
    _TEST_FLOAT(__prec_math_pow(three, 2.0), 9.0);
    _Fpu_WriteRound(_FPU_UPWARD);
    _TEST_FLOAT(__prec_math_pow(nine, 0.5), 3.0);
    _Fpu_WriteRound(_FPU_TOWARDZERO);
    _TEST_FLOAT(__prec_math_pow(-three, 3.0), -27.0);
    _Fpu_WriteRound(_FPU_UPWARD);
    _TEST_FLOAT(__prec_math_pow(two, -1074.0), 0x1p-1074);
    _Fpu_WriteRound(_FPU_DOWNWARD);
    _TEST_FLOAT(__prec_math_pow(thirteen, 11.0), 0x1.a145156b35p+40);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _Fpu_WriteRound(_FPU_TOWARDZERO);
    _TEST_FLOAT(__prec_math_pow(two, 1024.0), 0x1.fffffffffffffp+1023);
    _TEST_INT(_Fpu_TestFlags(_FPU_OVERFLOW), _FPU_OVERFLOW);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathExactPowers();
    return _Test_Status();
}
