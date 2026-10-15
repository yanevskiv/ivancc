// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> sin.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// sin should give the sine of x in radians within a few ulps in each
// direction and type, of the largest long doubles too, keep a zero's sign
// raising nothing, give a subnormal x itself with underflow, and a NaN with
// invalid and EDOM of an infinity.
// [C99:7.12.4.6p2]:  sin, of x in radians
// [C99:7.12.1p2]:    a domain error, of an infinity by footnote 203
// [C99:F.9.1.6p1]:   sin of a zero, invalid of an infinity
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathSin(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef = 0x1.d18f6ep-1f;
    static const long double valuel = 0xb.301c733f168e24ep-4L;
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double hugel = 0x1p16000L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(sinf(twof), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(sinl(hugel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_TRUE(within_ulps(sin(half), 0x1.eaee8744b05fp-2, DBL_MANT_DIG, ULPS));
    _TEST_TRUE(within_ulps(sin(1e300), -0x1.a2c16b010e385p-1, DBL_MANT_DIG, ULPS));

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(sin(0.0), 0.0);
    _TEST_FLOAT(sinf(-0.0f), -0.0f);
    _TEST_FLOAT(sinl(-0.0L), -0.0L);
    _TEST_TRUE(isnan(sin(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(sin(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(sin(inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathSin();
    return _Test_Status();
}
