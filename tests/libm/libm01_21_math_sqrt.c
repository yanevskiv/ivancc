// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> sqrt.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// sqrt should give the square root correctly rounded in the current
// direction in each type, exact where it is, keep a zero's sign and an
// infinity, and give a NaN with EDOM and invalid below zero.
// [C99:7.12.7.5p2]: sqrt, a domain error below zero
// [C99:7.12.1p2]:   a domain error, EDOM and invalid
// [C99:F.3p1]:      sqrt, IEC 60559's square root
// [C99:F.9.4.5p1]:  sqrt, fully specified by IEC 60559
static void Test_MathSqrt(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double root[MODES] = {0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0};
    static const float rootf[MODES] = {0x1.6a09e6p+0f, 0x1.6a09e6p+0f, 0x1.6a09e8p+0f, 0x1.6a09e6p+0f};
    static const long double rootl[MODES] = {0xb.504f333f9de6484p-3L, 0xb.504f333f9de6484p-3L, 0xb.504f333f9de6485p-3L, 0xb.504f333f9de6484p-3L};
    volatile double two = 2.0;
    volatile float twof = 2.0f;
    volatile long double twol = 2.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(sqrt(two), root[i]);
        _TEST_FLOAT(sqrtf(twof), rootf[i]);
        _TEST_FLOAT(sqrtl(twol), rootl[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(sqrt(144.0), 12.0);
    _TEST_FLOAT(sqrt(0x1p-1074), 0x1p-537);
    _TEST_FLOAT(sqrtf(0x1p-148f), 0x1p-74f);
    _TEST_FLOAT(sqrtf(0x1.21p+0f), 0x1.1p+0f);
    _TEST_FLOAT(sqrtl(0x1p-16444L), 0x1p-8222L);
    _TEST_FLOAT(sqrtl(4611686022722355201.0L), 2147483649.0L);
    _TEST_FLOAT(sqrt(-0.0), -0.0);
    _TEST_FLOAT(sqrtl(0.0L), 0.0L);
    _TEST_FLOAT(sqrt(inf), inf);
    _TEST_TRUE(isnan(sqrt(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_TRUE(isnan(sqrt(-1.0)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(sqrtf((float) -inf)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(sqrtl(-0x1p-16445L)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathSqrt();
    return _Test_Status();
}
