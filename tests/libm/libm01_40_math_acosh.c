// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> acosh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// acosh should give the nonnegative inverse hyperbolic cosine of x within a
// few ulps in each direction and type, +0 of 1 and +inf of +inf raising
// nothing, and a NaN with invalid and EDOM below 1.
// [C99:7.12.5.1p2]:  acosh, a domain error below 1
// [C99:7.12.5.1p3]:  acosh, in [0, +inf]
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:F.9.2.1p1]:   acosh of 1 and +inf, invalid below 1
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathAcosh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.ecc2caec5160ap-1;
    static const float valuef = 0x1.ecc2cap-1f;
    static const long double valuel = 0xb.17217f7d1cf79acp-4L;
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double quarterl = 1.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(acosh(half), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(acoshf(halff), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(acoshl(quarterl), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(acosh(1.0), 0.0);
    _TEST_FLOAT(acoshf(1.0f), 0.0f);
    _TEST_FLOAT(acoshl(1.0L), 0.0L);
    _TEST_FLOAT(acosh(inf), inf);
    _TEST_TRUE(isnan(acosh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_TRUE(isnan(acosh(0.5)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(acoshf((float) -inf)));
    _TEST_INT(errno, EDOM);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAcosh();
    return _Test_Status();
}
