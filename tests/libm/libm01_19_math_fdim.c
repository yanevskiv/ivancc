// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> fdim.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// fdim should give x - y when x > y and +0 otherwise in each type, a NaN of
// a NaN, and overflow to HUGE_VAL with ERANGE and overflow.
// [C99:7.12.12.1p2]: fdim, x - y if x > y, else +0
// [C99:7.12.1p4]:    an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9.9.1p1]:   fdim, no requirement beyond C99's
static void Test_MathFdim(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fdim(5.0, 3.0), 2.0);
    _TEST_FLOAT(fdim(3.0, 5.0), 0.0);
    _TEST_FLOAT(fdim(-3.0, -5.0), 2.0);
    _TEST_FLOAT(fdim(-0.0, 0.0), 0.0);
    _TEST_FLOAT(fdim(-5.0, -3.0), 0.0);
    _TEST_FLOAT(fdim(inf, inf), 0.0);
    _TEST_FLOAT(fdim(inf, 1.0), inf);
    _TEST_FLOAT(fdim(0x1p-1074, -0x1p-1074), 0x1p-1073);
    _TEST_TRUE(isnan(fdim(dnan, 1.0)));
    _TEST_TRUE(isnan(fdim(1.0, dnan)));
    _TEST_FLOAT(fdimf(1.5f, -FLT_MIN), 1.5f);
    _TEST_FLOAT(fdimf(-1.0f, 1.0f), 0.0f);
    _TEST_FLOAT(fdiml(1.0L, 0x1p-63L), 0x1.fffffffffffffffcp-1L);
    _TEST_FLOAT(fdiml(-LDBL_MAX, LDBL_MAX), 0.0L);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT & ~FE_INEXACT), 0);

    errno = 0;
    _TEST_FLOAT(fdim(DBL_MAX, -DBL_MAX), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_OVERFLOW), FE_OVERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(fdimf(FLT_MAX, -FLT_MAX), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(fdiml(LDBL_MAX, -LDBL_MAX), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
}

int main(void)
{
    Test_MathFdim();
    return _Test_Status();
}
