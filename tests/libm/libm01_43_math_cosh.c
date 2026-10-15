// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> cosh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// cosh should give the hyperbolic cosine of x within a few ulps in each
// direction and type, 1 of a zero and +inf of an infinity raising nothing,
// and a range error when it overflows.
// [C99:7.12.5.4p2]:  cosh, a range error if x's magnitude is too large
// [C99:7.12.1p4]:    an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9.2.4p1]:   cosh of a zero and an infinity
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathCosh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.080ab05ca6146p+0;
    static const float valuef = 0x1.2d1bc2p+1f;
    static const long double valuel = 0x8.405582e530a2f6ep-3L;
    volatile double quarter = 0.25;
    volatile float halff = -1.5f;
    volatile long double quarterl = -0.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(cosh(quarter), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(coshf(halff), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(coshl(quarterl), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(cosh(0.0), 1.0);
    _TEST_FLOAT(coshf(-0.0f), 1.0f);
    _TEST_FLOAT(coshl(-inf), HUGE_VALL);
    _TEST_FLOAT(cosh(inf), inf);
    _TEST_TRUE(isnan(cosh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(cosh(-800.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(coshf(90.0f), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(coshl(12000.0L), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathCosh();
    return _Test_Status();
}
