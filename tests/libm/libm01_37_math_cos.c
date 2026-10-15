// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> cos.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// cos should give the cosine of x in radians in each type, in the current
// direction for float and long double, 1 of a zero raising nothing, the
// cosine of a large x, and a NaN with invalid and EDOM of an infinity.
// [C99:7.12.4.5p2]: cos, of x in radians
// [C99:7.12.1p2]:   a domain error, of an infinity by footnote 203
// [C99:F.9.1.5p1]:  cos of a zero, invalid of an infinity
static void Test_MathCos(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.aa2266p-2f, -0x1.aa2266p-2f, -0x1.aa2264p-2f, -0x1.aa2264p-2f};
    static const long double valuel[MODES] = {0xf.80aa4fbef750ba8p-4L, 0xf.80aa4fbef750ba7p-4L, 0xf.80aa4fbef750ba8p-4L, 0xf.80aa4fbef750ba7p-4L};
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double quarterl = 0.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(cosf(twof), valuef[i]);
        _TEST_FLOAT(cosl(quarterl), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_FLOAT(cos(half), 0x1.c1528065b7d5p-1);
    _TEST_FLOAT(cos(1e22), 0x1.0be2cef01c8f4p-1);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(cos(0.0), 1.0);
    _TEST_FLOAT(cosf(-0.0f), 1.0f);
    _TEST_FLOAT(cosl(-0.0L), 1.0L);
    _TEST_TRUE(isnan(cos(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_TRUE(isnan(cos(-inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(cosl(inf)));
    _TEST_INT(errno, EDOM);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathCos();
    return _Test_Status();
}
