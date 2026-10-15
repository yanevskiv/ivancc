// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_atan.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_atan should give the arc tangent of x in [-pi/2, +pi/2] in each
// type, in the current direction for float and long double, keep a zero's sign
// raising nothing, give a tiny x itself and +-pi/2 of an infinity.
static void Test_PrecMathAtan(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.3fc176p+0f, -0x1.3fc178p+0f, -0x1.3fc176p+0f, -0x1.3fc176p+0f};
    static const long double valuel[MODES] = {0xb.c4de960b0029c17p-3L, 0xb.c4de960b0029c17p-3L, 0xb.c4de960b0029c18p-3L, 0xb.c4de960b0029c17p-3L};
    volatile double half = 0.5;
    volatile float threef = -3.0f;
    volatile long double tenl = 10.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_atanf(threef), valuef[i]);
        _TEST_FLOAT(__prec_math_atanl(tenl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_atan(half), 0x1.dac670561bb4fp-2);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_atan(0.0), 0.0);
    _TEST_FLOAT(__prec_math_atanf(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_atanl(-0.0L), -0.0L);
    _TEST_TRUE(_Exact_IsNan(__prec_math_atan(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _TEST_FLOAT(__prec_math_atan(inf), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(__prec_math_atanf((float) -inf), -0x1.921fb6p+0f);
    _TEST_FLOAT(__prec_math_atanl(0x1p100L), 0xc.90fdaa22168c235p-3L);
    _TEST_FLOAT(__prec_math_atan(-0x1p-70), -0x1p-70);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAtan();
    return _Test_Status();
}
