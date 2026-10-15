// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> tgamma.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define ULPS 4

// tgamma should give gamma of x within a few ulps in each type, a factorial
// exactly, +-inf with divide-by-zero of a zero, a domain error of a negative
// integer and of -inf, and a range error when its result overflows or
// underflows to zero.
// [C99:7.12.8.4p2]:  tgamma, a domain or range error of a negative integer or 0
// [C99:F.9.5.4p1]:   tgamma, a zero divide-by-zero, a negative integer invalid
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
static void Test_MathTgamma(void)
{
    volatile double half = 0.5;
    volatile float halff = 0.5f;
    volatile long double halfl = 0.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    _TEST_TRUE(within_ulps(tgamma(half), 0x1.c5bf891b4ef6bp+0, DBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgammaf(halff), 0x1.c5bf8ap+0f, FLT_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgammal(halfl), 0xe.2dfc48da77b553dp-3L, LDBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgamma(-2.5), -0x1.e3ff812e32183p-1, DBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgammaf(-2.5f), -0x1.e3ff82p-1f, FLT_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgammal(-2.5L), -0xf.1ffc097190c16a7p-4L, LDBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgamma(30.5), 0x1.305adf049c818p+105, DBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(tgammal(30.5L), 0x9.82d6f824e40bfa2p+102L, LDBL_MANT_DIG, ULPS));
    _TEST_FLOAT(tgamma(5.0), 24.0);
    _TEST_FLOAT(tgammaf(14.0f), 6227020800.0f);
    _TEST_FLOAT(tgammal(27.0L), 403291461126605635584000000.0L);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(tgamma(inf), inf);
    _TEST_TRUE(isnan(tgamma(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(tgamma(zero), HUGE_VAL);
    _TEST_FLOAT(tgammaf(-0.0f), -HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(tgamma(-1.0)));
    _TEST_TRUE(isnan(tgammal(-inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(tgamma(172.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(tgamma(-200.5), -0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathTgamma();
    return _Test_Status();
}
