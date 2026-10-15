// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> asinh.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// asinh should give the inverse hyperbolic sine of x in the current
// direction in each type, keep a zero's and an infinity's sign raising
// nothing, and give a subnormal x itself with underflow.
// [C99:7.12.5.2p2]: asinh, the arc hyperbolic sine
// [C99:F.9.2.2p1]:  asinh of a zero and an infinity
static void Test_MathAsinh(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.27fdfac4e438p+1, 0x1.27fdfac4e438p+1, 0x1.27fdfac4e4381p+1, 0x1.27fdfac4e438p+1};
    static const float valuef[MODES] = {0x1.31dcp+0f, 0x1.31dcp+0f, 0x1.31dc02p+0f, 0x1.31dcp+0f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double five = 5.0;
    volatile float halff = 1.5f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(asinh(five), value[i]);
        _TEST_FLOAT(asinhf(halff), valuef[i]);
        _TEST_FLOAT(asinhl(threel), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(asinh(0.0), 0.0);
    _TEST_FLOAT(asinhf(-0.0f), -0.0f);
    _TEST_FLOAT(asinhl(-inf), -HUGE_VALL);
    _TEST_FLOAT(asinh(inf), inf);
    _TEST_TRUE(isnan(asinh(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(asinh(0x1p-1074), 0x1p-1074);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAsinh();
    return _Test_Status();
}
