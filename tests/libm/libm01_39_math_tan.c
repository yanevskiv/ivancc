// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> tan.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// tan should give the tangent of x in radians in each type, in the current
// direction for float and long double, keep a zero's sign raising nothing,
// and give a NaN with invalid and EDOM of an infinity.
// [C99:7.12.4.7p2]: tan, of x in radians
// [C99:7.12.1p2]:   a domain error, of an infinity by footnote 203
// [C99:F.9.1.7p1]:  tan of a zero, invalid of an infinity
static void Test_MathTan(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.17af62p+1f, -0x1.17af64p+1f, -0x1.17af62p+1f, -0x1.17af62p+1f};
    static const long double valuel[MODES] = {0x9.1f7b892a5c37866p-6L, 0x9.1f7b892a5c37866p-6L, 0x9.1f7b892a5c37867p-6L, 0x9.1f7b892a5c37866p-6L};
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double threel = -3.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(tanf(twof), valuef[i]);
        _TEST_FLOAT(tanl(threel), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_FLOAT(tan(half), 0x1.17b4f5bf3474ap-1);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(tan(0.0), 0.0);
    _TEST_FLOAT(tanf(-0.0f), -0.0f);
    _TEST_FLOAT(tanl(-0.0L), -0.0L);
    _TEST_TRUE(isnan(tan(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_TRUE(isnan(tan(inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathTan();
    return _Test_Status();
}
