// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> gamma accuracy.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// lgammal next to a pole should be correctly rounded downward.
static void Test_PrecMathGammaAccuracy(void)
{
    volatile long double near_pole = -0x8.7fffffffffffffep+2L;

    _Fpu_WriteRound(_FPU_DOWNWARD);
    _TEST_FLOAT(__prec_math_lgammal(near_pole), -0xc.449271bd28ed47p+2L);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathGammaAccuracy();
    return _Test_Status();
}
