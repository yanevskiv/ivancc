// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> isfinite, isinf, isnan and isnormal.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// isfinite, isinf, isnan and isnormal should each hold of its own classes
// alone, in each type, evaluate their argument once and raise nothing.
// [C99:7.12.3.2p3]: isfinite, nonzero iff its argument is finite
// [C99:7.12.3.3p3]: isinf, nonzero iff its argument is infinite
// [C99:7.12.3.4p3]: isnan, nonzero iff its argument is a NaN
// [C99:7.12.3.5p3]: isnormal, nonzero iff its argument is normal
static void Test_MathPredicates(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;
    double dnan = zero / zero;
    float fnan = fzero / fzero;
    long double lnan = lzero / lzero;
    int count = 0;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isfinite(0.0f));
    _TEST_TRUE(isfinite(0x1p-149f));
    _TEST_TRUE(isfinite(-FLT_MAX));
    _TEST_TRUE(! isfinite(1.0f / fzero));
    _TEST_TRUE(! isfinite(fnan));
    _TEST_TRUE(isfinite(DBL_MAX));
    _TEST_TRUE(! isfinite(-1.0 / zero));
    _TEST_TRUE(isfinite(0x1p-16445L));
    _TEST_TRUE(! isfinite(lnan));

    _TEST_TRUE(isinf(1.0f / fzero));
    _TEST_TRUE(isinf(-1.0 / zero));
    _TEST_TRUE(isinf(1.0L / lzero));
    _TEST_TRUE(! isinf(FLT_MAX));
    _TEST_TRUE(! isinf(dnan));
    _TEST_TRUE(! isinf(LDBL_MAX));

    _TEST_TRUE(isnan(fnan));
    _TEST_TRUE(isnan(-dnan));
    _TEST_TRUE(isnan(lnan));
    _TEST_TRUE(isnan(NAN));
    _TEST_TRUE(! isnan(1.0f / fzero));
    _TEST_TRUE(! isnan(0.0));
    _TEST_TRUE(! isnan(-0.0L));

    _TEST_TRUE(isnormal(1.0f));
    _TEST_TRUE(isnormal(-DBL_MIN));
    _TEST_TRUE(isnormal(LDBL_MIN));
    _TEST_TRUE(! isnormal(0.0f));
    _TEST_TRUE(! isnormal(0x1p-1074));
    _TEST_TRUE(! isnormal(0x1p-16445L));
    _TEST_TRUE(! isnormal(1.0 / zero));
    _TEST_TRUE(! isnormal(fnan));
    _TEST_INT(fetestexcept(FE_INVALID), 0);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_TRUE(isfinite((double) ++count));
    _TEST_TRUE(! isinf((double) ++count));
    _TEST_TRUE(! isnan((double) ++count));
    _TEST_TRUE(isnormal((double) ++count));
    _TEST_INT(count, 4);
}

int main(void)
{
    Test_MathPredicates();
    return _Test_Status();
}
