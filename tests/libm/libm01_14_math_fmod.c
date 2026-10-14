// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> fmod.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// fmod should give x - ny exactly in each type, with x's sign and less than
// y in magnitude, of subnormals and of exponents far apart, keep a zero and
// a finite x over an infinity, and give a NaN and invalid for an infinite
// x or a zero y.
// [C99:7.12.10.1p3]: fmod, x - ny, x's sign, less than y in magnitude
// [C99:F.9.7.1p1]:   fmod of a zero, of an infinite x or a zero y, by an infinity
static void Test_MathFmod(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fmod(5.5, 2.0), 1.5);
    _TEST_FLOAT(fmod(-5.5, 2.0), -1.5);
    _TEST_FLOAT(fmod(5.5, -2.0), 1.5);
    _TEST_FLOAT(fmod(-6.0, 3.0), -0.0);
    _TEST_FLOAT(fmod(1.0, 3.0), 1.0);
    _TEST_FLOAT(fmod(DBL_MAX, 3.0), 2.0);
    _TEST_FLOAT(fmod(0x1p1023, 0x1p-1074), 0.0);
    _TEST_FLOAT(fmod(0x1.cp-1072, 0x1p-1073), 0x1p-1074);
    _TEST_FLOAT(fmod(-0.0, 1.0), -0.0);
    _TEST_FLOAT(fmod(3.0, -inf), 3.0);
    _TEST_FLOAT(fmodf(1e30f, 7.0f), 1.0f);
    _TEST_FLOAT(fmodf(-0x1p-149f, 0x1p-148f), -0x1p-149f);
    _TEST_FLOAT(fmodl(LDBL_MAX, 3.0L), 0.0L);
    _TEST_FLOAT(fmodl(0x1p16383L, 0x1.8p-16440L), 0x1p-16441L);
    _TEST_FLOAT(fmodl(-0x1.0000000000000002p0L, 0x1p-63L), -0.0L);
    _TEST_TRUE(isnan(fmod(dnan, 1.0)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT & ~FE_INEXACT), 0);

    _TEST_TRUE(isnan(fmod(inf, 1.0)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(fmodf(1.0f, 0.0f)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(fmodl(-1.0L, -0.0L)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathFmod();
    return _Test_Status();
}
