// (Test) Status: 0
// A test file for <_fp.h> formats against <float.h>'s.

// Includes.
#include <_test.h>
#include <float.h>
#include <_fp.h>

// The three formats _fp converts to should be float.h's.
static void Test_FpFormats(void)
{
    _Static_assert(FLT_MANT_DIG == _FP_FLOAT_MANT, "a float's mantissa");
    _Static_assert(FLT_MIN_EXP - 1 == _FP_FLOAT_MIN, "a float's least exponent");
    _Static_assert(FLT_MAX_EXP - 1 == _FP_FLOAT_MAX, "a float's greatest exponent");
    _Static_assert(DBL_MANT_DIG == _FP_DOUBLE_MANT, "a double's mantissa");
    _Static_assert(DBL_MIN_EXP - 1 == _FP_DOUBLE_MIN, "a double's least exponent");
    _Static_assert(DBL_MAX_EXP - 1 == _FP_DOUBLE_MAX, "a double's greatest exponent");
    _Static_assert(LDBL_MANT_DIG == _FP_LDOUBLE_MANT, "a long double's mantissa");
    _Static_assert(LDBL_MIN_EXP - 1 == _FP_LDOUBLE_MIN, "a long double's least exponent");
    _Static_assert(LDBL_MAX_EXP - 1 == _FP_LDOUBLE_MAX, "a long double's greatest exponent");
}

int main(void)
{
    Test_FpFormats();
    return _Test_Status();
}
