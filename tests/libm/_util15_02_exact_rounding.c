// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_exact.h> values and their rounding.

// Includes.
#include <_test.h>
#include <_exact.h>
#include <_fpu.h>

// 1 plus 2^-70, exact, should round once to each type in the current
// direction raising inexact, and its square root be developed exactly.
static void Test_ExactRounding(void)
{
    _Exact_Value one;
    _Exact_Value tiny;
    _Exact_Value sum;
    _Exact_Value root;

    _Exact_MakeValue(&one, 1.0L);
    _Exact_MakeValue(&tiny, 0x1p-70L);
    _Exact_AddValues(&sum, &one, &tiny);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_Exact_RoundValue(&sum, _EXACT_TYPE_DOUBLE), 1.0L);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_WriteRound(_FPU_UPWARD);
    _TEST_FLOAT(_Exact_RoundValue(&sum, _EXACT_TYPE_DOUBLE), 1.0L + 0x1p-52L);
    _TEST_FLOAT(_Exact_RoundValue(&sum, _EXACT_TYPE_FLOAT), 1.0L + 0x1p-23L);
    _TEST_FLOAT(_Exact_RoundValue(&sum, _EXACT_TYPE_LDOUBLE), 1.0L + 0x1p-63L);
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Exact_MakeValue(&sum, 2.25L);
    _Exact_ExtractRoot(&root, &sum, _EXACT_ROOT_SQUARE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_Exact_RoundValue(&root, _EXACT_TYPE_LDOUBLE), 1.5L);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
}

int main(void)
{
    Test_ExactRounding();
    return _Test_Status();
}
