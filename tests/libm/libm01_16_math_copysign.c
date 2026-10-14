// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> copysign.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// copysign should give x's magnitude y's sign in each type, of a zero, an
// infinity and a NaN too, and raise nothing.
// [C99:7.12.11.1p2]: copysign, x's magnitude and y's sign, of a NaN too
// [C99:F.9.8.1p1]:   copysign, as IEC 60559's
static void Test_MathCopysign(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;
    float fnan = (float) dnan;
    long double lnan = dnan;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(copysign(1.0, -0.0), -1.0);
    _TEST_FLOAT(copysign(-2.0, 3.0), 2.0);
    _TEST_FLOAT(copysign(0.0, -inf), -0.0);
    _TEST_FLOAT(copysign(-0x1p-1074, 1.0), 0x1p-1074);
    _TEST_FLOAT(copysign(1.0, -NAN), -1.0);
    _TEST_FLOAT(copysign(-1.0, NAN), 1.0);
    _TEST_TRUE(isnan(copysign(dnan, -1.0)));
    _TEST_INT(signbit(copysign(dnan, -1.0)) != 0, 1);
    _TEST_INT(signbit(copysign(-dnan, 1.0)), 0);
    _TEST_FLOAT(copysignf((float) inf, -1.0f), (float) -inf);
    _TEST_FLOAT(copysignf(-FLT_MIN, 0.0f), FLT_MIN);
    _TEST_INT(signbit(copysignf(fnan, -0.0f)) != 0, 1);
    _TEST_FLOAT(copysignl(LDBL_MAX, -1.0L), -LDBL_MAX);
    _TEST_FLOAT(copysignl(-0.0L, 0.0L), 0.0L);
    _TEST_INT(signbit(copysignl(lnan, -2.0L)) != 0, 1);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathCopysign();
    return _Test_Status();
}
