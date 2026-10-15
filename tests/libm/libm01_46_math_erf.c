// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> erf.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// erf should give the error function of x within a few ulps in each direction
// and type, keep a zero's sign, give +-1 of an infinity raising nothing, and
// give a subnormal x's value with underflow.
// [C99:7.12.8.1p2]:  erf, the error function
// [C99:F.9.5.1p1]:   erf of a zero and an infinity
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathErf(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.6c1c9759d0e5fp-1;
    static const float valuef = 0x1.6c1c98p-1f;
    static const long double valuel = 0xb.60e4bace872fb63p-4L;
    volatile double three = 0.75;
    volatile float threef = 0.75f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(erf(three), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(erff(threef), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(erfl(threel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(erf(0.0), 0.0);
    _TEST_FLOAT(erff(-0.0f), -0.0f);
    _TEST_FLOAT(erfl(-inf), -1.0L);
    _TEST_FLOAT(erf(inf), 1.0);
    _TEST_TRUE(isnan(erf(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(erf(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(erf(10.5), 1.0);
    _TEST_FLOAT(erfl(-10.5L), -1.0L);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathErf();
    return _Test_Status();
}
