// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> erfc.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4

// erfc should give 1 less the error function of x in the current direction
// in each type, 1 of a zero exactly, +0 and 2 of the infinities raising
// nothing, and a range error when its result underflows to zero.
// [C99:7.12.8.2p2]: erfc, 1 - erf x, a range error if x is too large
// [C99:F.9.5.2p1]:  erfc of an infinity
static void Test_MathErfc(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value[MODES] = {0x1.b82879728f11ep-1, 0x1.b82879728f11dp-1, 0x1.b82879728f11ep-1, 0x1.b82879728f11dp-1};
    static const float valuef[MODES] = {0x1.b8287ap-1f, 0x1.b82878p-1f, 0x1.b8287ap-1f, 0x1.b82878p-1f};
    static const long double valuel[MODES] = {0xd.c143cb94788ed18p-4L, 0xd.c143cb94788ed17p-4L, 0xd.c143cb94788ed18p-4L, 0xd.c143cb94788ed17p-4L};
    volatile double eighth = 0.125;
    volatile float eighthf = 0.125f;
    volatile long double eighthl = 0.125L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_FLOAT(erfc(eighth), value[i]);
        _TEST_FLOAT(erfcf(eighthf), valuef[i]);
        _TEST_FLOAT(erfcl(eighthl), valuel[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(erfc(0.0), 1.0);
    _TEST_FLOAT(erfcf(-0.0f), 1.0f);
    _TEST_FLOAT(erfcl(inf), 0.0L);
    _TEST_FLOAT(erfc(-inf), 2.0);
    _TEST_TRUE(isnan(erfc(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(erfc(30.0), 0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(erfcl(-30.0L), 2.0L);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathErfc();
    return _Test_Status();
}
