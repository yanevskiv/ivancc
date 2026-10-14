// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> log.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// log should give the natural logarithm in the current direction in each
// type, +0 of 1 in every direction, -inf with divide-by-zero and ERANGE of
// a zero, and a NaN with invalid and EDOM below zero.
// [C99:7.12.6.7p2]: log, a domain error below zero, a range error at zero
// [C99:7.12.1p2]:   a domain error, EDOM and invalid
// [C99:7.12.1p4]:   an exact infinity, HUGE_VAL, ERANGE and divide-by-zero
// [C99:F.9.3.7p1]:  log of a zero, 1, a negative and +inf
static void Test_MathLog(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.62e42fefa39efp-1, 0x1.62e42fefa39efp-1, 0x1.62e42fefa39fp-1, 0x1.62e42fefa39efp-1};
    static const float valuef[MODES] = {0x1.62e43p-1f, 0x1.62e42ep-1f, 0x1.62e43p-1f, 0x1.62e42ep-1f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double two = 2.0;
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double one = 1.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(log(two), value[i]);
        _TEST_FLOAT(logf(twof), valuef[i]);
        _TEST_FLOAT(logl(twol), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(log(one), 0.0);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(log(0x1p-1074), -0x1.74385446d71c3p+9);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(logl(inf), inf);
    _TEST_TRUE(isnan(log(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(log(zero), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(logf(-0.0f), -HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(log(-one)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(logl(-inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathLog();
    return _Test_Status();
}
