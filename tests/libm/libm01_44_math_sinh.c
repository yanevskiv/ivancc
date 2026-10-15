// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> sinh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// sinh should give the hyperbolic sine of x in the current direction in
// each type, keep a zero's and an infinity's sign raising nothing, give a
// subnormal x itself with underflow, and a range error when it overflows.
// [C99:7.12.5.5p2]: sinh, a range error if x's magnitude is too large
// [C99:7.12.1p4]:   an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9.2.5p1]:  sinh of a zero and an infinity
static void Test_MathSinh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.108c3aabd6a6p+1, 0x1.108c3aabd6a6p+1, 0x1.108c3aabd6a61p+1, 0x1.108c3aabd6a6p+1};
    static const float valuef[MODES] = {0x1.108c3ap+1f, 0x1.108c3ap+1f, 0x1.108c3cp+1f, 0x1.108c3ap+1f};
    static const long double valuel[MODES] = {0x8.8461d55eb530366p-2L, 0x8.8461d55eb530365p-2L, 0x8.8461d55eb530366p-2L, 0x8.8461d55eb530365p-2L};
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double halfl = 1.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(sinh(half), value[i]);
        _TEST_FLOAT(sinhf(halff), valuef[i]);
        _TEST_FLOAT(sinhl(halfl), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(sinh(0.0), 0.0);
    _TEST_FLOAT(sinhf(-0.0f), -0.0f);
    _TEST_FLOAT(sinhl(-inf), -HUGE_VALL);
    _TEST_FLOAT(sinh(inf), inf);
    _TEST_TRUE(isnan(sinh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(sinh(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(sinh(-800.0), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathSinh();
    return _Test_Status();
}
