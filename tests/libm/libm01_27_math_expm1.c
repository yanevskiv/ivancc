// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> expm1.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// expm1 should give e to x less 1 in the current direction in each type,
// without cancellation for a small x, keep a zero's sign, give -1 of -inf
// and +inf of +inf, and a range error when it overflows.
// [C99:7.12.6.3p2]: expm1, a range error if x is too large
// [C99:7.12.1p4]:   an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9.3.3p1]:  expm1 of a zero and an infinity
static void Test_MathExpm1(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.4c2531c3c0d38p-1, 0x1.4c2531c3c0d37p-1, 0x1.4c2531c3c0d38p-1, 0x1.4c2531c3c0d37p-1};
    static const float valuef[MODES] = {0x1.b7e152p+0f, 0x1.b7e15p+0f, 0x1.b7e152p+0f, 0x1.b7e15p+0f};
    static const long double valuel[MODES] = {-0xa.1d2a7274c4320e5p-4L, -0xa.1d2a7274c4320e6p-4L, -0xa.1d2a7274c4320e5p-4L, -0xa.1d2a7274c4320e5p-4L};
    volatile double half = 0.5;
    volatile float onef = 1.0f;
    volatile long double onel = -1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(expm1(half), value[i]);
        _TEST_FLOAT(expm1f(onef), valuef[i]);
        _TEST_FLOAT(expm1l(onel), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(expm1(0.0), 0.0);
    _TEST_FLOAT(expm1f(-0.0f), -0.0f);
    _TEST_FLOAT(expm1l(-inf), -1.0L);
    _TEST_FLOAT(expm1(inf), inf);
    _TEST_TRUE(isnan(expm1(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_FLOAT(expm1(0x1p-60), 0x1p-60);
    _TEST_FLOAT(expm1(-50.0), -1.0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(expm1(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(expm1(710.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(expm1f(89.0f), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(expm1l(11357.0L), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathExpm1();
    return _Test_Status();
}
