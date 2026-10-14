// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> cbrt.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// cbrt should give the real cube root correctly rounded in the current
// direction in each type, exact of a cube, a negative of a negative, and
// keep a zero's sign and an infinity.
// [C99:7.12.7.1p2]: cbrt, the real cube root
// [C99:F.9.4.1p1]:  cbrt of a zero and an infinity
static void Test_MathCbrt(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float rootf[MODES] = {0x1.428a3p+0f, 0x1.428a2ep+0f, 0x1.428a3p+0f, 0x1.428a2ep+0f};
    static const long double rootl[MODES] = {0xa.14517cc6b945711p-3L, 0xa.14517cc6b945711p-3L, 0xa.14517cc6b945712p-3L, 0xa.14517cc6b945711p-3L};
    static const long double negl[MODES] = {-0xa.14517cc6b945711p-3L, -0xa.14517cc6b945712p-3L, -0xa.14517cc6b945711p-3L, -0xa.14517cc6b945711p-3L};
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(cbrtf(twof), rootf[i]);
        _TEST_FLOAT(cbrtl(twol), rootl[i]);
        _TEST_FLOAT(cbrtl(-twol), negl[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(cbrtf(-8.0f), -2.0f);
    _TEST_FLOAT(cbrtl(1000.0L), 10.0L);
    _TEST_FLOAT(cbrtl(0x1p-16443L), 0x1p-5481L);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(cbrt(-0.0), -0.0);
    _TEST_FLOAT(cbrtf(0.0f), 0.0f);
    _TEST_FLOAT(cbrt(-inf), -inf);
    _TEST_FLOAT(cbrtl(inf), inf);
    _TEST_TRUE(isnan(cbrt(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathCbrt();
    return _Test_Status();
}
