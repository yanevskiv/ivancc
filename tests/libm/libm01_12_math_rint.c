// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> rint and nearbyint.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// rint and nearbyint should round to an integer in the current direction in
// each type, keep a zero's sign and an infinity, rint raising inexact when
// the value changes and nearbyint never.
// [C99:7.12.9.3p2]: nearbyint, in the current direction, raising no inexact
// [C99:7.12.9.4p2]: rint, nearbyint that may raise inexact
// [C99:F.9.6.3p1]:  nearbyint of a zero and an infinity, in every direction
// [C99:F.9.6.4p1]:  rint, inexact if the result differs
static void Test_MathRint(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double up[MODES] = {2.0, 2.0, 3.0, 2.0};
    static const double down[MODES] = {-2.0, -3.0, -2.0, -2.0};
    static const float small[MODES] = {-0.0f, -1.0f, -0.0f, -0.0f};
    static const long double big[MODES] = {0x1p62L, 0x1p62L, 4611686018427387905.0L, 0x1p62L};
    volatile double zero = 0.0;
    double inf = 1.0 / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(nearbyint(2.5), up[i]);
        _TEST_FLOAT(nearbyint(-2.5), down[i]);
        _TEST_FLOAT(nearbyintf(-0.25f), small[i]);
        _TEST_FLOAT(nearbyintl(4611686018427387904.5L), big[i]);
        _TEST_FLOAT(nearbyint(-0.0), -0.0);
        _TEST_FLOAT(nearbyint(-inf), -inf);
        _TEST_FLOAT(nearbyintl(LDBL_MAX), LDBL_MAX);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

        _TEST_FLOAT(rint(2.5), up[i]);
        _TEST_INT(fetestexcept(FE_INEXACT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(rint(-2.5), down[i]);
        _TEST_FLOAT(rintf(-0.25f), small[i]);
        _TEST_FLOAT(rintl(4611686018427387904.5L), big[i]);
        _TEST_INT(fetestexcept(FE_INEXACT), FE_INEXACT);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(rint(-3.0), -3.0);
        _TEST_FLOAT(rintf(0x1p100f), 0x1p100f);
        _TEST_FLOAT(rintl(-0.0L), -0.0L);
        _TEST_FLOAT(rint(inf), inf);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    }
    fesetround(FE_TONEAREST);
}

int main(void)
{
    Test_MathRint();
    return _Test_Status();
}
