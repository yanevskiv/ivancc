// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> atan.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// atan should give the arc tangent of x in [-pi/2, +pi/2] in each type, in
// the current direction for float and long double, keep a zero's sign
// raising nothing, give a tiny x itself and +-pi/2 of an infinity.
// [C99:7.12.4.3p2]: atan, the principal value
// [C99:7.12.4.3p3]: atan, in [-pi/2, +pi/2]
// [C99:F.9.1.3p1]:  atan of a zero and an infinity
static void Test_MathAtan(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.3fc176p+0f, -0x1.3fc178p+0f, -0x1.3fc176p+0f, -0x1.3fc176p+0f};
    static const long double valuel[MODES] = {0xb.c4de960b0029c17p-3L, 0xb.c4de960b0029c17p-3L, 0xb.c4de960b0029c18p-3L, 0xb.c4de960b0029c17p-3L};
    volatile double half = 0.5;
    volatile float threef = -3.0f;
    volatile long double tenl = 10.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(atanf(threef), valuef[i]);
        _TEST_FLOAT(atanl(tenl), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);
    _TEST_FLOAT(atan(half), 0x1.dac670561bb4fp-2);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(atan(0.0), 0.0);
    _TEST_FLOAT(atanf(-0.0f), -0.0f);
    _TEST_FLOAT(atanl(-0.0L), -0.0L);
    _TEST_TRUE(isnan(atan(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_FLOAT(atan(inf), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(atanf((float) -inf), -0x1.921fb6p+0f);
    _TEST_FLOAT(atanl(0x1p100L), 0xc.90fdaa22168c235p-3L);
    _TEST_FLOAT(atan(-0x1p-70), -0x1p-70);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathAtan();
    return _Test_Status();
}
