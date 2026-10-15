// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> acos.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// acos should give the arc cosine of x in [0, pi] within a few ulps in each
// direction and type, +0 of 1 raising nothing, pi/2 of a zero and pi of -1,
// and a NaN with invalid and EDOM outside [-1, +1], an infinity too.
// [C99:7.12.4.1p2]:  acos, a domain error outside [-1, +1]
// [C99:7.12.4.1p3]:  acos, in [0, pi]
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:F.9.1.1p1]:   acos(1) +0, invalid for |x| > 1
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathAcos(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef = 0x1.d2cf5cp+0f;
    static const long double valuel = 0x8.60a91c16b9b2c23p-2L;
    volatile double half = 0.5;
    volatile float quarterf = -0.25f;
    volatile long double halfl = -0.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(acosf(quarterf), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(acosl(halfl), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_TRUE(within_ulps(acos(half), 0x1.0c152382d7366p+0, DBL_MANT_DIG, ULPS));

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(acos(1.0), 0.0);
    _TEST_FLOAT(acosf(1.0f), 0.0f);
    _TEST_FLOAT(acosl(1.0L), 0.0L);
    _TEST_TRUE(isnan(acos(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_FLOAT(acos(-0.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(acos(-1.0), 0x1.921fb54442d18p+1);
    _TEST_FLOAT(acosl(0.0L), 0xc.90fdaa22168c235p-3L);

    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(acos(-1.5)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(acosf((float) inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAcos();
    return _Test_Status();
}
