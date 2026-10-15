// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_exact.h> long double fields.

// Includes.
#include <_test.h>
#include <_exact.h>

// The sign and class tests should read a long double's bits, taking the
// x87's refused encodings as NaNs, and the sign functions should write them.
static void Test_ExactFields(void)
{
    long double unnormal = _Exact_MakeLDouble(0x3FFFU, 0);
    long double pseudo_inf = _Exact_MakeLDouble(_EXACT_LDOUBLE_EXP_MAX, 0);
    long double inf = _Exact_MakeLDouble(_EXACT_LDOUBLE_EXP_MAX, _EXACT_LDOUBLE_INT_BIT);
    long double nan = _Exact_MakeLDouble(_EXACT_LDOUBLE_EXP_MAX, _EXACT_LDOUBLE_INT_BIT | 1U);
    int exp;

    _TEST_TRUE(_Exact_IsNan(nan));
    _TEST_TRUE(_Exact_IsNan(unnormal));
    _TEST_TRUE(_Exact_IsNan(pseudo_inf));
    _TEST_TRUE(! _Exact_IsNan(inf));
    _TEST_TRUE(! _Exact_IsNan(0x1p-16445L));
    _TEST_TRUE(_Exact_IsInf(inf));
    _TEST_TRUE(_Exact_IsInf(-inf));
    _TEST_TRUE(! _Exact_IsInf(pseudo_inf));
    _TEST_TRUE(_Exact_IsNegative(-0.0L));
    _TEST_TRUE(! _Exact_IsNegative(0.0L));

    _TEST_FLOAT(_Exact_ClearSign(-2.5L), 2.5L);
    _TEST_FLOAT(_Exact_CopySign(2.5L, -0.0L), -2.5L);
    _TEST_FLOAT(_Exact_CopySign(-2.5L, 1.0L), 2.5L);

    _TEST_UINT(_Exact_SplitLDouble(0x1p-16445L, &exp), _EXACT_LDOUBLE_INT_BIT);
    _TEST_INT(exp, -16445);
    _TEST_FLOAT(_Exact_JoinLDouble(exp, _EXACT_LDOUBLE_INT_BIT), 0x1p-16445L);
    _TEST_FLOAT(_Exact_StepLDouble(1.0L, _EXACT_STEP_UP), 1.0L + 0x1p-63L);
    _TEST_FLOAT(_Exact_StepLDouble(1.0L, _EXACT_STEP_DOWN), 1.0L - 0x1p-64L);
}

int main(void)
{
    Test_ExactFields();
    return _Test_Status();
}
