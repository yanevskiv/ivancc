// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> log2.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// log2 should give the base-2 logarithm within a few ulps in each
// direction and type, a power of 2's exponent, +0 of 1 in every direction,
// -inf with divide-by-zero and ERANGE of a zero, and a NaN with invalid
// and EDOM below zero.
// [C99:7.12.6.10p2]: log2, a domain error below zero, a range error at zero
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:7.12.1p4]:    an exact infinity, HUGE_VAL, ERANGE and divide-by-zero
// [C99:F.9.3.10p1]:  log2 of a zero, 1, a negative and +inf
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathLog2(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.95c01a39fbd68p+0;
    static const float valuef = 0x1.95c01ap+0f;
    static const long double valuel = 0xc.ae00d1cfdeb43dp-3L;
    volatile double three = 3.0;
    volatile float threef = 3.0f;
    volatile long double threel = 3.0L;
    volatile double one = 1.0;
    volatile double eight = 8.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(log2(three), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(log2f(threef), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(log2l(threel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(log2(one), 0.0);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(log2(eight), 3.0);
    _TEST_FLOAT(log2f(0x1p-149f), -149.0f);
    _TEST_FLOAT(log2l(0x1p-16445L), -16445.0L);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(log2(inf), inf);
    _TEST_TRUE(isnan(log2(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(log2(zero), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(log2l(-2.0L)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathLog2();
    return _Test_Status();
}
