// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> ilogb and logb.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// ilogb and logb should extract the exponent in each type, a subnormal's as
// if normalized, and give ilogb's three macros and logb's infinities, with
// invalid and divide-by-zero.
// [C99:7.12.6.5p2]:  ilogb, FP_ILOGB0, INT_MAX and FP_ILOGBNAN
// [C99:7.12.6.11p2]: logb, a subnormal as if normalized
// [C99:F.9.3.5p1]:   ilogb, invalid when the result is outside int
// [C99:F.9.3.11p1]:  logb of a zero, -inf and divide-by-zero, of an infinity +inf
static void Test_MathIlogb(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(ilogb(1.0), 0);
    _TEST_INT(ilogb(-0x1.fp-3), -3);
    _TEST_INT(ilogb(0x1p-1074), -1074);
    _TEST_INT(ilogb(DBL_MAX), 1023);
    _TEST_INT(ilogbf(0x1p-149f), -149);
    _TEST_INT(ilogbf(FLT_MAX), 127);
    _TEST_INT(ilogbl(0x1p-16445L), -16445);
    _TEST_INT(ilogbl(LDBL_MAX), 16383);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_INT(ilogb(0.0), FP_ILOGB0);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(ilogbf((float) -inf), INT_MAX);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(ilogbl(dnan), FP_ILOGBNAN);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_FLOAT(logb(8.0), 3.0);
    _TEST_FLOAT(logb(-1.5), 0.0);
    _TEST_FLOAT(logb(0x1p-1074), -1074.0);
    _TEST_FLOAT(logbf(0x1.8p-140f), -140.0f);
    _TEST_FLOAT(logbl(0x1p-16445L), -16445.0L);
    _TEST_FLOAT(logbl(-LDBL_MAX), 16383.0L);
    _TEST_FLOAT(logb(-inf), inf);
    _TEST_TRUE(isnan(logb(dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_FLOAT(logb(-0.0), -inf);
    _TEST_INT(fetestexcept(FE_DIVBYZERO), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(logbl(0.0L), -inf);
    _TEST_INT(fetestexcept(FE_DIVBYZERO), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathIlogb();
    return _Test_Status();
}
