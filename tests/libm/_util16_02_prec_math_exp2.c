// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_exp2.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_exp2 should give 2 to x in the current direction in each type, a power
// of 2 exactly, 1 of a zero, +0 and +inf of the infinities, and a range
// error when the result overflows to an infinity or underflows to zero.
static void Test_PrecMathExp2(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bccp+0};
    static const float valuef[MODES] = {0x1.6a09e6p+0f, 0x1.6a09e6p+0f, 0x1.6a09e8p+0f, 0x1.6a09e6p+0f};
    static const long double valuel[MODES] = {0xb.504f333f9de6484p-4L, 0xb.504f333f9de6484p-4L, 0xb.504f333f9de6485p-4L, 0xb.504f333f9de6484p-4L};
    volatile double half = 0.5;
    volatile float halff = 0.5f;
    volatile long double halfl = -0.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_exp2(half), value[i]);
        _TEST_FLOAT(_prec_exp2f(halff), valuef[i]);
        _TEST_FLOAT(_prec_exp2l(halfl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_exp2(3.0), 8.0);
    _TEST_FLOAT(_prec_exp2(-1022.0), 0x1p-1022);
    _TEST_FLOAT(_prec_exp2f(-149.0f), 0x1p-149f);
    _TEST_FLOAT(_prec_exp2l(16383.0L), 0x1p16383L);
    _TEST_FLOAT(_prec_exp2(-0.0), 1.0);
    _TEST_FLOAT(_prec_exp2f(0.0f), 1.0f);
    _TEST_FLOAT(_prec_exp2(-inf), 0.0);
    _TEST_FLOAT(_prec_exp2l(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_exp2(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_exp2(1024.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_exp2(-1080.0), 0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_exp2f(128.0f), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_exp2l(-16446.0L), 0.0L);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathExp2();
    return _Test_Status();
}
