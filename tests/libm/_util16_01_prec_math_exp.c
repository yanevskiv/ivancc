// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_exp.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_exp should give e to x in the current direction in each type, 1
// of a zero, +0 and +inf of the infinities, and a range error when the result
// overflows to an infinity or underflows to zero.
static void Test_PrecMathExp(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.5bf0a8b145769p+1, 0x1.5bf0a8b145769p+1, 0x1.5bf0a8b14576ap+1, 0x1.5bf0a8b145769p+1};
    static const float valuef[MODES] = {0x1.5bf0a8p+1f, 0x1.5bf0a8p+1f, 0x1.5bf0aap+1f, 0x1.5bf0a8p+1f};
    static const long double valuel[MODES] = {0xa.df85458a2bb4a9bp-2L, 0xa.df85458a2bb4a9ap-2L, 0xa.df85458a2bb4a9bp-2L, 0xa.df85458a2bb4a9ap-2L};
    volatile double one = 1.0;
    volatile float onef = 1.0f;
    volatile long double onel = 1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_exp(one), value[i]);
        _TEST_FLOAT(__prec_math_expf(onef), valuef[i]);
        _TEST_FLOAT(__prec_math_expl(onel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_exp(0.0), 1.0);
    _TEST_FLOAT(__prec_math_expf(-0.0f), 1.0f);
    _TEST_FLOAT(__prec_math_expl(-inf), 0.0L);
    _TEST_FLOAT(__prec_math_exp(inf), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_exp(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_exp(-740.0), 0x55p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_exp(-800.0), 0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_exp(710.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expf(89.0f), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expl(11357.0L), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_expl(-11400.0L), 0.0L);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _Fpu_WriteRound(_FPU_TOWARDZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_exp(710.0), 0x1.fffffffffffffp+1023);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_WriteRound(_FPU_TONEAREST);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathExp();
    return _Test_Status();
}
