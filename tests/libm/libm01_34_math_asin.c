// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> asin.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// asin should give the arc sine of x in [-pi/2, +pi/2] within a few ulps in
// each direction and type, keep a zero's sign raising nothing, give pi/2 of
// 1, a tiny x itself, and a NaN with invalid and EDOM outside [-1, +1].
// [C99:7.12.4.2p2]:  asin, a domain error outside [-1, +1]
// [C99:7.12.4.2p3]:  asin, in [-pi/2, +pi/2]
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:F.9.1.2p1]:   asin of a zero, invalid for |x| > 1
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathAsin(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef = -0x1.02be9cp-2f;
    static const long double valuel = 0xd.91a98ae3406e041p-4L;
    volatile double half = 0.5;
    volatile float quarterf = -0.25f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(asinf(quarterf), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(asinl(threel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_TRUE(within_ulps(asin(half), 0x1.0c152382d7366p-1, DBL_MANT_DIG, ULPS));

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(asin(0.0), 0.0);
    _TEST_FLOAT(asinf(-0.0f), -0.0f);
    _TEST_FLOAT(asinl(-0.0L), -0.0L);
    _TEST_TRUE(isnan(asin(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_FLOAT(asin(1.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(asinf(-1.0f), -0x1.921fb6p+0f);
    _TEST_FLOAT(asin(0x1p-70), 0x1p-70);

    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(asin(2.0)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(asinl(-1.0000001L)));
    _TEST_INT(errno, EDOM);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAsin();
    return _Test_Status();
}
