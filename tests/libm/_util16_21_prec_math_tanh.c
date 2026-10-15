// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_tanh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_tanh should give the hyperbolic tangent of x in the current direction
// in each type, keep a zero's sign and give +-1 of an infinity raising
// nothing, and give a subnormal x itself with underflow.
static void Test_PrecMathTanh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.cf6f9786df577p-1, 0x1.cf6f9786df577p-1, 0x1.cf6f9786df578p-1, 0x1.cf6f9786df577p-1};
    static const float valuef[MODES] = {0x1.cf6f98p-1f, 0x1.cf6f96p-1f, 0x1.cf6f98p-1f, 0x1.cf6f96p-1f};
    static const long double valuel[MODES] = {0xe.7b7cbc36fabbb07p-4L, 0xe.7b7cbc36fabbb06p-4L, 0xe.7b7cbc36fabbb07p-4L, 0xe.7b7cbc36fabbb06p-4L};
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double halfl = 1.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_tanh(half), value[i]);
        _TEST_FLOAT(_prec_tanhf(halff), valuef[i]);
        _TEST_FLOAT(_prec_tanhl(halfl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_tanh(0.0), 0.0);
    _TEST_FLOAT(_prec_tanhf(-0.0f), -0.0f);
    _TEST_FLOAT(_prec_tanhl(-inf), -1.0L);
    _TEST_FLOAT(_prec_tanh(inf), 1.0);
    _TEST_TRUE(_Exact_IsNan(_prec_tanh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_tanh(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_tanh(30.0), 1.0);
    _TEST_FLOAT(_prec_tanh(-30.0), -1.0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathTanh();
    return _Test_Status();
}
