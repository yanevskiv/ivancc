// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> fma.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// fma should give x times y plus z rounded once in the current direction in
// each type, never twice through a wider one, with no intermediate overflow,
// an exact zero's sign by the direction, and invalid of an infinity times a
// zero unless z is a NaN.
// [C99:7.12.13.1p2]: fma, x times y plus z, rounded once in the current mode
// [C99:F.9.10.1p1]:  fma, invalid of an infinity times a zero, optional by a NaN
static void Test_MathFma(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double up[MODES] = {1.0, 1.0, 0x1.0000000000001p+0, 1.0};
    static const float upf[MODES] = {1.0f, 1.0f, 0x1.000002p+0f, 1.0f};
    static const long double upl[MODES] = {1.0L, 1.0L, 0x1.0000000000000002p+0L, 1.0L};
    static const double cancel[MODES] = {0.0, -0.0, 0.0, 0.0};
    volatile double one = 1.0;
    volatile float onef = 1.0f;
    volatile long double onel = 1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(fma(one, one, 0x1p-60), up[i]);
        _TEST_FLOAT(fmaf(onef, onef, 0x1p-30f), upf[i]);
        _TEST_FLOAT(fmal(onel, onel, 0x1p-70L), upl[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(fma(one, one, -one), cancel[i]);
        _TEST_FLOAT(fmal(-onel, onel, onel), (long double) cancel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fma(1 + 0x1p-52, 1 - 0x1p-52, -1.0), -0x1p-104);
    _TEST_FLOAT(fmal(1 + 0x1p-63L, 1 - 0x1p-63L, -1.0L), -0x1p-126L);
    _TEST_FLOAT(fma(DBL_MAX, 2.0, -DBL_MAX), DBL_MAX);
    _TEST_FLOAT(fmaf(FLT_MAX, -2.0f, FLT_MAX), -FLT_MAX);
    _TEST_FLOAT(fmal(LDBL_MAX, 2.0L, -LDBL_MAX), LDBL_MAX);
    _TEST_FLOAT(fma(DBL_MIN, 0x1.8p-3, 0.0), 0x1.8p-1025);
    _TEST_FLOAT(fma(-0.0, 1.0, 0.0), 0.0);
    _TEST_FLOAT(fma(-0.0, 1.0, -0.0), -0.0);
    _TEST_FLOAT(fmaf(0.0f, -1.0f, 3.0f), 3.0f);
    _TEST_FLOAT(fma(DBL_MAX, 2.0, -inf), -inf);
    _TEST_TRUE(isnan(fma(inf, zero, dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_FLOAT(fmaf(1 + 0x1p-12f, 1 + 0x1p-12f, 0x1p-60f), 0x1.002002p+0f);
    _TEST_FLOAT(fma(1 + 0x1p-27, 1 + 0x1p-26, 0x1p-80), 0x1.0000006000001p+0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);

    errno = 0;
    _TEST_TRUE(isnan(fma(inf, zero, 1.0)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(fmal(0.0L, inf, 1.0L)));
    _TEST_TRUE(isnan(fma(inf, 1.0, -inf)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fma(DBL_MAX, 2.0, 0.0), HUGE_VAL);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fmal(-LDBL_MAX, 2.0L, 0.0L), -HUGE_VALL);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fma(0x1p-1074, 0.5, 0.0), 0.0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fmaf(FLT_MIN, -FLT_MIN, 0.0f), -0.0f);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(errno, 0);
}

int main(void)
{
    Test_MathFma();
    return _Test_Status();
}
