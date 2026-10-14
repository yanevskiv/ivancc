// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> hypot.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// hypot should give the square root of the sum of the squares correctly
// rounded in the current direction in each type, with no undue overflow or
// underflow, +inf of an infinity even with a NaN, and ERANGE on overflow.
// [C99:7.12.7.3p2]: hypot, without undue overflow or underflow
// [C99:7.12.1p4]:   an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9.4.3p1]:  hypot of a zero, and of an infinity even with a NaN
static void Test_MathHypot(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double root[MODES] = {0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0};
    static const float rootf[MODES] = {0x1.6a09e6p+0f, 0x1.6a09e6p+0f, 0x1.6a09e8p+0f, 0x1.6a09e6p+0f};
    volatile double one = 1.0;
    volatile float onef = 1.0f;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(hypot(one, -one), root[i]);
        _TEST_FLOAT(hypotf(-onef, onef), rootf[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(hypotl(1.0L, 1.0L), 0xb.504f333f9de6484p-3L);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(hypot(-3.0, 4.0), 5.0);
    _TEST_FLOAT(hypotf(5.0f, -12.0f), 13.0f);
    _TEST_FLOAT(hypotl(20.0L, 21.0L), 29.0L);
    _TEST_FLOAT(hypotl(0x1.8p+16382L, 0x1p+16383L), 0x1.4p+16383L);
    _TEST_FLOAT(hypot(-0x1p-1074, 0.0), 0x1p-1074);
    _TEST_FLOAT(hypotf(-0.0f, -0.0f), 0.0f);
    _TEST_FLOAT(hypot(inf, dnan), inf);
    _TEST_FLOAT(hypotl(dnan, -inf), inf);
    _TEST_TRUE(isnan(hypot(dnan, 1.0)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(hypot(0x1p-1074, 0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(hypot(DBL_MAX, 1.0), DBL_MAX);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(hypot(DBL_MAX, DBL_MAX), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(hypotf(FLT_MAX, -FLT_MAX), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(hypotl(LDBL_MAX, LDBL_MAX), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathHypot();
    return _Test_Status();
}
