// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> absolute values.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// fabs, fabsf and fabsl should give a magnitude, of a zero and infinity too.
// [C99:7.12.7.2p2]: the fabs functions compute the absolute value of x
// [C99:F.9.4.2]: fabs(+-0) is +0, fabs(+-infinity) is +infinity
static void Test_MathFabs(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;

    _TEST_FLOAT(fabs(-1.5), 1.5);
    _TEST_FLOAT(fabs(2.25), 2.25);
    _TEST_FLOAT(fabs(-0.0), 0.0);
    _TEST_FLOAT(fabs(-0x1p-1074), 0x1p-1074);
    _TEST_FLOAT(fabs(-1.0 / zero), 1.0 / zero);
    _TEST_TRUE(fabs(zero / zero) != fabs(zero / zero));
    _TEST_FLOAT(fabsf(-1.5f), 1.5f);
    _TEST_FLOAT(fabsf(-0.0f), 0.0f);
    _TEST_FLOAT(fabsf(-0x1p-149f), 0x1p-149f);
    _TEST_FLOAT(fabsf(-1.0f / fzero), 1.0f / fzero);
    _TEST_TRUE(fabsf(fzero / fzero) != fabsf(fzero / fzero));
    _TEST_FLOAT(fabsl(-1.5L), 1.5L);
    _TEST_FLOAT(fabsl(-0.0L), 0.0L);
    _TEST_FLOAT(fabsl(-0x1p-16445L), 0x1p-16445L);
    _TEST_FLOAT(fabsl(-1.0L / lzero), 1.0L / lzero);
    _TEST_TRUE(fabsl(lzero / lzero) != fabsl(lzero / lzero));
}

int main(void)
{
    Test_MathFabs();
    return _Test_Status();
}
