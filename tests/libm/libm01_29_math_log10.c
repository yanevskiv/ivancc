// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> log10.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// log10 should give the base-10 logarithm in the current direction in each
// type, a power of 10's exponent, +0 of 1 in every direction, -inf with
// divide-by-zero and ERANGE of a zero, and a NaN with invalid and EDOM
// below zero.
// [C99:7.12.6.8p2]: log10, a domain error below zero, a range error at zero
// [C99:7.12.1p2]:   a domain error, EDOM and invalid
// [C99:7.12.1p4]:   an exact infinity, HUGE_VAL, ERANGE and divide-by-zero
// [C99:F.9.3.8p1]:  log10 of a zero, 1, a negative and +inf
static void Test_MathLog10(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.34413509f79ffp-2, 0x1.34413509f79fep-2, 0x1.34413509f79ffp-2, 0x1.34413509f79fep-2};
    static const float valuef[MODES] = {0x1.344136p-2f, 0x1.344134p-2f, 0x1.344136p-2f, 0x1.344134p-2f};
    static const long double valuel[MODES] = {0x9.a209a84fbcff799p-5L, 0x9.a209a84fbcff798p-5L, 0x9.a209a84fbcff799p-5L, 0x9.a209a84fbcff798p-5L};
    volatile double two = 2.0;
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double one = 1.0;
    volatile double thousand = 1000.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(log10(two), value[i]);
        _TEST_FLOAT(log10f(twof), valuef[i]);
        _TEST_FLOAT(log10l(twol), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(log10(one), 0.0);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(log10(thousand), 3.0);
    _TEST_FLOAT(log10f(1e10f), 10.0f);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(log10(inf), inf);
    _TEST_TRUE(isnan(log10l(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(log10(-zero), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(log10f(-1.0f)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathLog10();
    return _Test_Status();
}
