// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> tanh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// tanh should give the hyperbolic tangent of x within a few ulps in each
// direction and type, keep a zero's sign and give +-1 of an infinity raising
// nothing, and give a subnormal x itself with underflow.
// [C99:7.12.5.6p2]:  tanh, the hyperbolic tangent
// [C99:F.9.2.6p1]:   tanh of a zero and an infinity
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathTanh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.cf6f9786df577p-1;
    static const float valuef = 0x1.cf6f98p-1f;
    static const long double valuel = 0xe.7b7cbc36fabbb07p-4L;
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double halfl = 1.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(tanh(half), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(tanhf(halff), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(tanhl(halfl), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(tanh(0.0), 0.0);
    _TEST_FLOAT(tanhf(-0.0f), -0.0f);
    _TEST_FLOAT(tanhl(-inf), -1.0L);
    _TEST_FLOAT(tanh(inf), 1.0);
    _TEST_TRUE(isnan(tanh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(tanh(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(tanh(30.0), 1.0);
    _TEST_FLOAT(tanh(-30.0), -1.0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathTanh();
    return _Test_Status();
}
