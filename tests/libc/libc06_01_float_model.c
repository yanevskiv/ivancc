// (Test) Status: 0
// A test file for <float.h> radix, rounding and evaluation method.

// Include helpers.
#include "libc06_00_float_helpers.inc"

// [C99:S5.2.4.2.2p6] The radix, evaluation and DECIMAL_DIG should be 2, 0 and 21.
#define DIRECTIVE_MODEL 0
#if (FLT_RADIX == 2) \
    && (FLT_EVAL_METHOD == 0) \
    && (DECIMAL_DIG == 21)
#undef DIRECTIVE_MODEL
#define DIRECTIVE_MODEL 1
#endif

// [C99:S5.2.4.2.2p9] The radix should be 2, sums round to nearest, none run wider.
static void Test_FloatModel(void)
{
    _TEST_INT(FLT_RADIX, 2);
    _TEST_INT(FLT_ROUNDS, 1);
    _TEST_INT(FLT_EVAL_METHOD, 0);
    _TEST_INT(DIRECTIVE_MODEL, 1);
    _TEST_UINT(sizeof(float), 4);
    _TEST_UINT(sizeof(double), 8);
    _TEST_UINT(sizeof(long double), 16);
}

int main(void)
{
    Test_FloatModel();
    return _Test_Status();
}
