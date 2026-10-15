// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> atan2.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// atan2 should give the arc tangent of y/x in [-pi, +pi] by the signs of
// both within a few ulps in each direction and type, Annex F's results of
// zeros and infinities, raising nothing of zeros, and a range error when the
// result underflows to zero.
// [C99:7.12.4.4p2]:  atan2, the quadrant by both signs
// [C99:7.12.4.4p3]:  atan2, in [-pi, +pi]
// [C99:7.12.1p5]:    an underflow, ERANGE implementation-defined
// [C99:F.9.1.4p1]:   atan2 of zeros and infinities, footnote 322
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathAtan2(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef = -0x1.4978fap-1f;
    static const long double valuel = -0x9.6cbe3f9990e91a8p-2L;
    volatile double one = 1.0;
    volatile float threef = -3.0f;
    volatile long double onel = -1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(atan2f(threef, 4.0f), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(atan2l(onel, onel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_TRUE(within_ulps(atan2(one, -2.0), 0x1.56c6e7397f5aep+1, DBL_MANT_DIG, ULPS));

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(atan2(0.0, 0.0), 0.0);
    _TEST_FLOAT(atan2(-0.0, 0.0), -0.0);
    _TEST_FLOAT(atan2f(-0.0f, 2.0f), -0.0f);
    _TEST_FLOAT(atan2l(0.0L, inf), 0.0L);
    _TEST_FLOAT(atan2(-1.0, inf), -0.0);
    _TEST_TRUE(isnan(atan2(dnan, 1.0)));
    _TEST_TRUE(isnan(atan2(inf, dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_FLOAT(atan2(0.0, -0.0), 0x1.921fb54442d18p+1);
    _TEST_FLOAT(atan2(-0.0, -0.0), -0x1.921fb54442d18p+1);
    _TEST_FLOAT(atan2(-0.0, -1.0), -0x1.921fb54442d18p+1);
    _TEST_FLOAT(atan2(1.0, -inf), 0x1.921fb54442d18p+1);
    _TEST_FLOAT(atan2(1.0, 0.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(atan2(-1.0, -0.0), -0x1.921fb54442d18p+0);
    _TEST_FLOAT(atan2(inf, 1.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(atan2(-inf, -inf), -0x1.2d97c7f3321d2p+1);
    _TEST_FLOAT(atan2f((float) inf, (float) inf), 0x1.921fb6p-1f);
    _TEST_FLOAT(atan2l(-inf, inf), -0xc.90fdaa22168c235p-4L);

    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(atan2(0x1p-1000, 0x1p100), 0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAtan2();
    return _Test_Status();
}
