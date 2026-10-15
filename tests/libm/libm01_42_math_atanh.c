// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> atanh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// atanh should give the inverse hyperbolic tangent of x within a few ulps in
// each direction and type, keep a zero's sign raising nothing, give a pole at
// +-1 with divide-by-zero and ERANGE, and a NaN with invalid and EDOM outside
// [-1, +1].
// [C99:7.12.5.3p2]:  atanh, a domain error outside [-1, +1]
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:7.12.1p4]:    an exact infinity, HUGE_VAL, ERANGE and divide-by-zero
// [C99:F.9.2.3p1]:   atanh of a zero, +-1 and past them
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathAtanh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.f2272ae325a57p-1;
    static const float valuef = 0x1.193ea8p-1f;
    volatile double three = 0.75;
    volatile float halff = 0.5f;
    volatile long double fifthl = 0.2L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(atanh(three), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(atanhf(halff), valuef, FLT_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_TRUE(within_ulps(atanhl(fifthl), 0xc.f991f65fcc25f96p-6L, LDBL_MANT_DIG, ULPS));

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(atanh(0.0), 0.0);
    _TEST_FLOAT(atanhf(-0.0f), -0.0f);
    _TEST_FLOAT(atanhl(-0.0L), -0.0L);
    _TEST_TRUE(isnan(atanh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(atanh(1.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(atanhl(-1.0L), -HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(atanh(1.5)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(atanhf((float) inf)));
    _TEST_INT(errno, EDOM);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAtanh();
    return _Test_Status();
}
