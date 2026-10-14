// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> exp2.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// exp2 should give 2 to x in the current direction in each type, a power
// of 2 exactly, 1 of a zero, +0 and +inf of the infinities, and a range
// error when the result overflows to an infinity or underflows to zero.
// [C99:7.12.6.2p2]: exp2, a range error if x's magnitude is too large
// [C99:7.12.1p4]:   an overflow, HUGE_VAL, ERANGE and overflow
// [C99:7.12.1p5]:   an underflow, ERANGE implementation-defined
// [C99:F.9.3.2p1]:  exp2 of a zero and an infinity
static void Test_MathExp2(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0};
    static const float valuef[MODES] = {0x1.6a09e6p+0f, 0x1.6a09e6p+0f, 0x1.6a09e8p+0f, 0x1.6a09e6p+0f};
    static const long double valuel[MODES] = {0xb.504f333f9de6484p-4L, 0xb.504f333f9de6484p-4L, 0xb.504f333f9de6485p-4L, 0xb.504f333f9de6484p-4L};
    volatile double half = 0.5;
    volatile float halff = 0.5f;
    volatile long double halfl = -0.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(exp2(half), value[i]);
        _TEST_FLOAT(exp2f(halff), valuef[i]);
        _TEST_FLOAT(exp2l(halfl), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(exp2(3.0), 8.0);
    _TEST_FLOAT(exp2(-1022.0), 0x1p-1022);
    _TEST_FLOAT(exp2f(-149.0f), 0x1p-149f);
    _TEST_FLOAT(exp2l(16383.0L), 0x1p16383L);
    _TEST_FLOAT(exp2(-0.0), 1.0);
    _TEST_FLOAT(exp2f(0.0f), 1.0f);
    _TEST_FLOAT(exp2(-inf), 0.0);
    _TEST_FLOAT(exp2l(inf), inf);
    _TEST_TRUE(isnan(exp2(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(exp2(1024.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(exp2(-1080.0), 0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(exp2f(128.0f), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(exp2l(-16446.0L), 0.0L);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathExp2();
    return _Test_Status();
}
