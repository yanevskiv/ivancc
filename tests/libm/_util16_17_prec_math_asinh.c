// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_asinh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_asinh should give the inverse hyperbolic sine of x in the current
// direction in each type, keep a zero's and an infinity's sign raising
// nothing, and give a subnormal x itself with underflow.
static void Test_PrecMathAsinh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.27fdfac4e438p+1, 0x1.27fdfac4e438p+1, 0x1.27fdfac4e4381p+1, 0x1.27fdfac4e438p+1};
    static const float valuef[MODES] = {0x1.31dcp+0f, 0x1.31dcp+0f, 0x1.31dc02p+0f, 0x1.31dcp+0f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double five = 5.0;
    volatile float halff = 1.5f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_asinh(five), value[i]);
        _TEST_FLOAT(_prec_asinhf(halff), valuef[i]);
        _TEST_FLOAT(_prec_asinhl(threel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_asinh(0.0), 0.0);
    _TEST_FLOAT(_prec_asinhf(-0.0f), -0.0f);
    _TEST_FLOAT(_prec_asinhl(-inf), -_EXACT_INFINITY);
    _TEST_FLOAT(_prec_asinh(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_asinh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_asinh(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAsinh();
    return _Test_Status();
}
