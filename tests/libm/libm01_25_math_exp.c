// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> exp.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// exp should give e to x in the current direction in each type, 1 of a
// zero, +0 and +inf of the infinities, and a range error when the result
// overflows to an infinity or underflows to zero.
// [C99:7.12.6.1p2]: exp, a range error if x's magnitude is too large
// [C99:7.12.1p4]:   an overflow, HUGE_VAL, ERANGE and overflow
// [C99:7.12.1p5]:   an underflow, ERANGE implementation-defined
// [C99:F.9.3.1p1]:  exp of a zero and an infinity
static void Test_MathExp(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.5bf0a8b145769p+1, 0x1.5bf0a8b145769p+1, 0x1.5bf0a8b14576ap+1, 0x1.5bf0a8b145769p+1};
    static const float valuef[MODES] = {0x1.5bf0a8p+1f, 0x1.5bf0a8p+1f, 0x1.5bf0aap+1f, 0x1.5bf0a8p+1f};
    static const long double valuel[MODES] = {0xa.df85458a2bb4a9bp-2L, 0xa.df85458a2bb4a9ap-2L, 0xa.df85458a2bb4a9bp-2L, 0xa.df85458a2bb4a9ap-2L};
    volatile double one = 1.0;
    volatile float onef = 1.0f;
    volatile long double onel = 1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(exp(one), value[i]);
        _TEST_FLOAT(expf(onef), valuef[i]);
        _TEST_FLOAT(expl(onel), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(exp(0.0), 1.0);
    _TEST_FLOAT(expf(-0.0f), 1.0f);
    _TEST_FLOAT(expl(-inf), 0.0L);
    _TEST_FLOAT(exp(inf), inf);
    _TEST_TRUE(isnan(exp(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(exp(-740.0), 0x55p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(exp(-800.0), 0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(exp(710.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    errno = 0;
    _TEST_FLOAT(expf(89.0f), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(expl(11357.0L), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(expl(-11400.0L), 0.0L);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    fesetround(FE_TOWARDZERO);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(exp(710.0), DBL_MAX);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    fesetround(FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathExp();
    return _Test_Status();
}
