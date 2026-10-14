// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> fmax and fmin.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// fmax and fmin should give the greater and the lesser in each type, the
// other argument of one quiet NaN and a NaN of two, and raise nothing.
// [C99:7.12.12.2p2]: fmax, the maximum numeric value, its footnote 213
// [C99:7.12.12.3p2]: fmin, the minimum numeric value, its footnote 214
// [C99:F.9.9.2p1]:   fmax, the other argument of one NaN, a NaN of two
// [C99:F.9.9.3p1]:   fmin, as fmax
static void Test_MathFmax(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;
    float fnan = (float) dnan;
    long double lnan = dnan;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(fmax(1.0, 2.0), 2.0);
    _TEST_FLOAT(fmax(-1.0, -2.0), -1.0);
    _TEST_FLOAT(fmax(-inf, 0x1p-1074), 0x1p-1074);
    _TEST_FLOAT(fmax(dnan, -3.0), -3.0);
    _TEST_FLOAT(fmax(-3.0, dnan), -3.0);
    _TEST_TRUE(isnan(fmax(dnan, dnan)));
    _TEST_FLOAT(fmaxf(fnan, (float) inf), (float) inf);
    _TEST_FLOAT(fmaxf(FLT_MIN, -FLT_MAX), FLT_MIN);
    _TEST_FLOAT(fmaxl(1.0L, 1.0L + 0x1p-63L), 1.0L + 0x1p-63L);
    _TEST_FLOAT(fmaxl(lnan, -LDBL_MAX), -LDBL_MAX);

    _TEST_FLOAT(fmin(1.0, 2.0), 1.0);
    _TEST_FLOAT(fmin(-inf, 0x1p-1074), -inf);
    _TEST_FLOAT(fmin(dnan, -3.0), -3.0);
    _TEST_FLOAT(fmin(-3.0, dnan), -3.0);
    _TEST_TRUE(isnan(fmin(dnan, dnan)));
    _TEST_FLOAT(fminf(2.0f, fnan), 2.0f);
    _TEST_FLOAT(fminf(-FLT_MIN, 0x1p-149f), -FLT_MIN);
    _TEST_FLOAT(fminl(1.0L, 1.0L - 0x1p-64L), 1.0L - 0x1p-64L);
    _TEST_FLOAT(fminl(LDBL_MAX, lnan), LDBL_MAX);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathFmax();
    return _Test_Status();
}
