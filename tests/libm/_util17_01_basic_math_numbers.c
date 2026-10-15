// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_basic_math.h> numbers against C99's.

// Includes.
#include <_test.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <_basic_math.h>

// C99's classes, ilogb's results, int's limits and the least normal values
// should be the ones _basic_math has.
static void Test_BasicMathNumbers(void)
{
    _Static_assert(FP_INFINITE == _BASIC_MATH_FP_INFINITE, "FP_INFINITE should be _BASIC_MATH_FP_INFINITE");
    _Static_assert(FP_NAN == _BASIC_MATH_FP_NAN, "FP_NAN should be _BASIC_MATH_FP_NAN");
    _Static_assert(FP_NORMAL == _BASIC_MATH_FP_NORMAL, "FP_NORMAL should be _BASIC_MATH_FP_NORMAL");
    _Static_assert(FP_SUBNORMAL == _BASIC_MATH_FP_SUBNORMAL, "FP_SUBNORMAL should be _BASIC_MATH_FP_SUBNORMAL");
    _Static_assert(FP_ZERO == _BASIC_MATH_FP_ZERO, "FP_ZERO should be _BASIC_MATH_FP_ZERO");
    _Static_assert(FP_ILOGB0 == _BASIC_MATH_FP_ILOGB0, "FP_ILOGB0 should be _BASIC_MATH_FP_ILOGB0");
    _Static_assert(FP_ILOGBNAN == _BASIC_MATH_FP_ILOGBNAN, "FP_ILOGBNAN should be _BASIC_MATH_FP_ILOGBNAN");
    _Static_assert(INT_MAX == _BASIC_MATH_INT_MAX, "INT_MAX should be _BASIC_MATH_INT_MAX");
    _Static_assert(INT_MIN == _BASIC_MATH_INT_MIN, "INT_MIN should be _BASIC_MATH_INT_MIN");
    _TEST_TRUE(FLT_MIN == _BASIC_MATH_FLT_MIN);
    _TEST_TRUE(DBL_MIN == _BASIC_MATH_DBL_MIN);
    _TEST_TRUE(LDBL_MIN == _BASIC_MATH_LDBL_MIN);
}

int main(void)
{
    Test_BasicMathNumbers();
    return _Test_Status();
}
