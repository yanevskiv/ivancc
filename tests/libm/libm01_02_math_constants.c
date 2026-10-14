// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> constants and types.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// NAN should be a constant expression, so it can initialize a static.
static const float nan_static = NAN;

// HUGE_VAL and INFINITY should be positive infinities and NAN a quiet NaN,
// each of its type and constant; the classes distinct; ilogb's two INT_MIN;
// and errors reported both by errno and by the flags.
// [C99:7.12p2]: float_t and double_t, float and double, FLT_EVAL_METHOD 0
// [C99:7.12p3]: HUGE_VAL, a double; HUGE_VALF and HUGE_VALL, its analogs
// [C99:7.12p4]: INFINITY, a constant expression of float, an infinity
// [C99:7.12p5]: NAN, a constant expression of float, a quiet NaN
// [C99:7.12p6]: the number classification macros, distinct
// [C99:7.12p8]: FP_ILOGB0, INT_MIN or -INT_MAX; FP_ILOGBNAN, INT_MAX or
//               INT_MIN
// [C99:7.12p9]: MATH_ERRNO 1, MATH_ERREXCEPT 2, math_errhandling of them
// [C99:F.9p2]:  HUGE_VAL and its analogs, positive infinities
// [C99:F.9p4]:  math_errhandling & MATH_ERREXCEPT, nonzero
static void Test_MathConstants(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;
    uint32_t bits;

    _TEST_UINT(sizeof(HUGE_VAL), sizeof(double));
    _TEST_UINT(sizeof(HUGE_VALF), sizeof(float));
    _TEST_UINT(sizeof(HUGE_VALL), sizeof(long double));
    _TEST_UINT(sizeof(INFINITY), sizeof(float));
    _TEST_UINT(sizeof(NAN), sizeof(float));
    _TEST_FLOAT(HUGE_VAL, 1.0 / zero);
    _TEST_FLOAT(HUGE_VALF, 1.0f / fzero);
    _TEST_FLOAT(HUGE_VALL, 1.0L / lzero);
    _TEST_FLOAT(INFINITY, 1.0f / fzero);
    memcpy(&bits, &nan_static, sizeof(bits));
    _TEST_UINT(bits, 0x7FC00000);
    feclearexcept(FE_ALL_EXCEPT);
    float nan_auto = NAN;
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    memcpy(&bits, &nan_auto, sizeof(bits));
    _TEST_UINT(bits, 0x7FC00000);

    _TEST_INT(FP_NAN, 0);
    _TEST_INT(FP_INFINITE, 1);
    _TEST_INT(FP_ZERO, 2);
    _TEST_INT(FP_SUBNORMAL, 3);
    _TEST_INT(FP_NORMAL, 4);
    _TEST_INT(FP_ILOGB0, INT_MIN);
    _TEST_INT(FP_ILOGBNAN, INT_MIN);
    _TEST_INT(MATH_ERRNO, 1);
    _TEST_INT(MATH_ERREXCEPT, 2);
    _TEST_INT(math_errhandling, MATH_ERRNO | MATH_ERREXCEPT);
    _TEST_UINT(sizeof(float_t), sizeof(float));
    _TEST_UINT(sizeof(double_t), sizeof(double));
}

int main(void)
{
    Test_MathConstants();
    return _Test_Status();
}
