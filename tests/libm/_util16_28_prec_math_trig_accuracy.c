// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> trigonometric and hyperbolic accuracy.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// cos and tan of the double nearest a multiple of pi/2 and sinhl should be
// correctly rounded to nearest, and cos and tanh of a tiny x should round
// the right way in a directed mode.
static void Test_PrecMathTrigAccuracy(void)
{
    volatile double reduced = 0x1.6ac5b262ca1ffp+849;
    volatile long double hyperbolic = 0xf.bf9f3d87bff0cap-10L;
    volatile double tiny = 0x1p-30;
    volatile double least = 0x1p-1022;

    _TEST_FLOAT(__prec_math_cos(reduced), -0x1.14ae72e6ba22fp-61);
    _TEST_FLOAT(__prec_math_tan(reduced), -0x1.d9ba9a7975636p+60);
    _TEST_FLOAT(__prec_math_sinhl(hyperbolic), 0xf.bfc7ed594726667p-10L);
    _Fpu_WriteRound(_FPU_DOWNWARD);
    _TEST_FLOAT(__prec_math_cos(tiny), 0x1.fffffffffffffp-1);
    _Fpu_WriteRound(_FPU_UPWARD);
    _TEST_FLOAT(__prec_math_tanh(least), 0x1p-1022);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathTrigAccuracy();
    return _Test_Status();
}
