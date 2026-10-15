// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_log1p.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_log1p should give the natural logarithm of 1 plus x in the
// current direction in each type, without cancellation for a small x, keep a
// zero's sign, give -inf with divide-by-zero and ERANGE of -1, and a NaN with
// invalid and EDOM below -1.
static void Test_PrecMathLog1p(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.62e42fefa39efp-1, 0x1.62e42fefa39efp-1, 0x1.62e42fefa39fp-1, 0x1.62e42fefa39efp-1};
    static const double neg[MODES] = {-0x1.62e42fefa39efp-1, -0x1.62e42fefa39fp-1, -0x1.62e42fefa39efp-1, -0x1.62e42fefa39efp-1};
    static const float valuef[MODES] = {0x1.62e43p-1f, 0x1.62e42ep-1f, 0x1.62e43p-1f, 0x1.62e42ep-1f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double one = 1.0;
    volatile double half = -0.5;
    volatile float onef = 1.0f;
    volatile long double onel = 1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_log1p(one), value[i]);
        _TEST_FLOAT(__prec_math_log1p(half), neg[i]);
        _TEST_FLOAT(__prec_math_log1pf(onef), valuef[i]);
        _TEST_FLOAT(__prec_math_log1pl(onel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_log1p(0.0), 0.0);
    _TEST_FLOAT(__prec_math_log1pf(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_log1pl(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_log1p(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _TEST_FLOAT(__prec_math_log1p(0x1p-60), 0x1p-60);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_log1p(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_log1p(-one), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_log1p(-2.0)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_log1pl(-inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathLog1p();
    return _Test_Status();
}
