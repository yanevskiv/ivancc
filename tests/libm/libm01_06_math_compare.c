// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> comparison macros.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Each comparison macro should give its operator's value, false of any NaN
// but isunordered, in each type and across types, evaluate each argument
// once, and raise no invalid for a quiet NaN.
// [C99:7.12.14p1]:   quiet versions of the relational operators
// [C99:7.12.14.1p2]: isgreater, (x) > (y), raising no invalid if unordered
// [C99:7.12.14.2p2]: isgreaterequal, (x) >= (y), the same
// [C99:7.12.14.3p2]: isless, (x) < (y), the same
// [C99:7.12.14.4p2]: islessequal, (x) <= (y), the same
// [C99:7.12.14.5p2]: islessgreater, (x) < (y) || (x) > (y), x and y once
// [C99:7.12.14.6p3]: isunordered, 1 if unordered, else 0
static void Test_MathCompare(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;
    double dnan = zero / zero;
    float fnan = fzero / fzero;
    long double lnan = lzero / lzero;
    int count = 0;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isgreater(2.0, 1.0));
    _TEST_TRUE(! isgreater(1.0f, 1.0f));
    _TEST_TRUE(isgreaterequal(1.0L, 1.0L));
    _TEST_TRUE(! isgreaterequal(1.0, 2.0f));
    _TEST_TRUE(isless(-1.0 / zero, -DBL_MAX));
    _TEST_TRUE(! isless(0.0, -0.0));
    _TEST_TRUE(islessequal(0.0f, -0.0f));
    _TEST_TRUE(! islessequal(0x1p-1073, 0x1p-1074));
    _TEST_TRUE(islessgreater(1.0f, 2.0L));
    _TEST_TRUE(! islessgreater(0.0L, -0.0L));
    _TEST_TRUE(isless(1.0f, 1.0f + 0x1p-23f));
    _TEST_TRUE(isless(1.0L, 1.0L + 0x1p-63L));
    _TEST_TRUE(isgreater(0x1p-149f, 0.0));
    _TEST_INT(isunordered(1.0, 2.0), 0);
    _TEST_INT(fetestexcept(FE_INVALID), 0);

    _TEST_TRUE(! isgreater(dnan, 1.0));
    _TEST_TRUE(! isgreaterequal(fnan, fnan));
    _TEST_TRUE(! isless(1.0L, lnan));
    _TEST_TRUE(! islessequal(dnan, 1.0f / fzero));
    _TEST_TRUE(! islessgreater(fnan, 0.0));
    _TEST_INT(isunordered(dnan, 1.0), 1);
    _TEST_INT(isunordered(1.0f, fnan), 1);
    _TEST_INT(isunordered(lnan, lnan), 1);
    _TEST_INT(isunordered(NAN, 0.0f), 1);
    _TEST_INT(fetestexcept(FE_INVALID), 0);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_TRUE(isless((double) ++count, 2.0));
    _TEST_TRUE(islessgreater((double) ++count, 0.0));
    _TEST_INT(count, 2);
}

int main(void)
{
    Test_MathCompare();
    return _Test_Status();
}
