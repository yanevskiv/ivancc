// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> modf.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// modf should break a value into its integral and fractional parts in each
// type, each with the value's sign, an infinity's fraction a zero, and
// raise nothing.
// [C99:7.12.6.12p2]: modf, the integral and fractional parts, x's sign each
// [C99:F.9.3.12p1]:  modf of an infinity, a zero and the infinity, of a NaN
static void Test_MathModf(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;
    long double lwhole = 0.0L;
    double whole = 0.0;
    float fwhole = 0.0f;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(modf(3.25, &whole), 0.25);
    _TEST_FLOAT(whole, 3.0);
    _TEST_FLOAT(modf(-3.25, &whole), -0.25);
    _TEST_FLOAT(whole, -3.0);
    _TEST_FLOAT(modf(-2.0, &whole), -0.0);
    _TEST_FLOAT(whole, -2.0);
    _TEST_FLOAT(modf(-0x1p-1074, &whole), -0x1p-1074);
    _TEST_FLOAT(whole, -0.0);
    _TEST_FLOAT(modf(-inf, &whole), -0.0);
    _TEST_FLOAT(whole, -inf);
    _TEST_TRUE(isnan(modf(dnan, &whole)));
    _TEST_TRUE(isnan(whole));

    _TEST_FLOAT(modff(0x1.fffffep22f, &fwhole), 0.5f);
    _TEST_FLOAT(fwhole, 8388607.0f);
    _TEST_FLOAT(modff(0x1p30f, &fwhole), 0.0f);
    _TEST_FLOAT(fwhole, 0x1p30f);

    _TEST_FLOAT(modfl(2305843009213693952.75L, &lwhole), 0.75L);
    _TEST_FLOAT(lwhole, 2305843009213693952.0L);
    _TEST_FLOAT(modfl(-0.5L, &lwhole), -0.5L);
    _TEST_FLOAT(lwhole, -0.0L);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathModf();
    return _Test_Status();
}
