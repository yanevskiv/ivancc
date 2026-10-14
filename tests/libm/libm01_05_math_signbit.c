// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> signbit.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// signbit should be nonzero for a negative sign alone, of a zero, an
// infinity and a NaN too, in each type, evaluate its argument once and raise
// nothing.
// [C99:7.12.3.6p3]: signbit, nonzero iff its argument's sign is negative
// [C99:7.12.3.6p2]: signbit, of infinities, zeros and NaNs too
static void Test_MathSignbit(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;
    int count = 0;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(signbit(-1.0f));
    _TEST_TRUE(signbit(-0.0f));
    _TEST_TRUE(signbit(-1.0f / fzero));
    _TEST_TRUE(! signbit(0.0f));
    _TEST_TRUE(! signbit(1.0f / fzero));
    _TEST_TRUE(! signbit(NAN));
    _TEST_TRUE(signbit(-NAN));
    _TEST_TRUE(signbit(-0x1p-1074));
    _TEST_TRUE(signbit(-0.0));
    _TEST_TRUE(! signbit(0.0));
    _TEST_TRUE(! signbit(1.0 / zero));
    _TEST_TRUE(signbit(-0.0L));
    _TEST_TRUE(signbit(-1.0L / lzero));
    _TEST_TRUE(! signbit(LDBL_MAX));
    _TEST_TRUE(signbit(-(long double) NAN));
    _TEST_INT(fetestexcept(FE_INVALID), 0);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_TRUE(! signbit((double) ++count));
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_MathSignbit();
    return _Test_Status();
}
