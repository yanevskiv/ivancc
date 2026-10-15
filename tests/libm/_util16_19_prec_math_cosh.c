// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_cosh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_cosh should give the hyperbolic cosine of x in the current direction in
// each type, 1 of a zero and +inf of an infinity raising nothing, and a
// range error when it overflows.
static void Test_PrecMathCosh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.080ab05ca6146p+0, 0x1.080ab05ca6145p+0, 0x1.080ab05ca6146p+0, 0x1.080ab05ca6145p+0};
    static const float valuef[MODES] = {0x1.2d1bc2p+1f, 0x1.2d1bc2p+1f, 0x1.2d1bc4p+1f, 0x1.2d1bc2p+1f};
    static const long double valuel[MODES] = {0x8.405582e530a2f6ep-3L, 0x8.405582e530a2f6ep-3L, 0x8.405582e530a2f6fp-3L, 0x8.405582e530a2f6ep-3L};
    volatile double quarter = 0.25;
    volatile float halff = -1.5f;
    volatile long double quarterl = -0.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_cosh(quarter), value[i]);
        _TEST_FLOAT(_prec_coshf(halff), valuef[i]);
        _TEST_FLOAT(_prec_coshl(quarterl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_cosh(0.0), 1.0);
    _TEST_FLOAT(_prec_coshf(-0.0f), 1.0f);
    _TEST_FLOAT(_prec_coshl(-inf), _EXACT_INFINITY);
    _TEST_FLOAT(_prec_cosh(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_cosh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_cosh(-800.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_coshf(90.0f), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_coshl(12000.0L), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathCosh();
    return _Test_Status();
}
