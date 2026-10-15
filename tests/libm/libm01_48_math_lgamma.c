// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> lgamma.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// lgamma should give the natural logarithm of |gamma| of x within a few ulps in
// each direction and type, +0 of 1 and 2 exactly, +inf of the infinities
// raising nothing, +inf with divide-by-zero of a pole, and a range error when
// its result overflows.
// [C99:7.12.8.3p2]:  lgamma, ln |gamma x|, a range error if x is too large
// [C99:F.9.5.3p1]:   lgamma of 1, 2 and the infinities, of a pole divide-by-zero
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathLgamma(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.2383e809a67e8p-2;
    static const float valuef = 0x1.2383e8p-2f;
    static const long double valuel = 0x9.1c1f404d33f3cfcp-5L;
    volatile double five = 2.5;
    volatile float fivef = 2.5f;
    volatile long double fivel = 2.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(lgamma(five), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(lgammaf(fivef), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(lgammal(fivel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(lgamma(-0.75), 0x1.93616060ea5ep+0);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(lgamma(1.0), 0.0);
    _TEST_FLOAT(lgammaf(2.0f), 0.0f);
    _TEST_FLOAT(lgammal(2.0L), 0.0L);
    _TEST_FLOAT(lgamma(inf), inf);
    _TEST_FLOAT(lgammal(-inf), inf);
    _TEST_TRUE(isnan(lgamma(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(lgamma(zero), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(lgammal(-3.0L), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(lgamma(DBL_MAX), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathLgamma();
    return _Test_Status();
}
