// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> log1p.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// log1p should give the natural logarithm of 1 plus x within a few ulps
// in each direction and type, without cancellation for a small x, keep a
// zero's sign, give -inf with divide-by-zero and ERANGE of -1, and a NaN
// with invalid and EDOM below -1.
// [C99:7.12.6.9p2]:  log1p, of 1 plus x, a domain error below -1
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:7.12.1p4]:    an exact infinity, HUGE_VAL, ERANGE and divide-by-zero
// [C99:F.9.3.9p1]:   log1p of a zero, -1, below -1 and +inf
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathLog1p(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.62e42fefa39efp-1;
    static const double neg = -0x1.62e42fefa39efp-1;
    static const float valuef = 0x1.62e43p-1f;
    static const long double valuel = 0xb.17217f7d1cf79acp-4L;
    volatile double one = 1.0;
    volatile double half = -0.5;
    volatile float onef = 1.0f;
    volatile long double onel = 1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(log1p(one), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(log1p(half), neg, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(log1pf(onef), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(log1pl(onel), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(log1p(0.0), 0.0);
    _TEST_FLOAT(log1pf(-0.0f), -0.0f);
    _TEST_FLOAT(log1pl(inf), inf);
    _TEST_TRUE(isnan(log1p(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_FLOAT(log1p(0x1p-60), 0x1p-60);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(log1p(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(log1p(-one), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(log1p(-2.0)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(log1pl(-inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathLog1p();
    return _Test_Status();
}
