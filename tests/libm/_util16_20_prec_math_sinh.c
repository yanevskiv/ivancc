// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_sinh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_sinh should give the hyperbolic sine of x in the current direction in
// each type, keep a zero's and an infinity's sign raising nothing, give a
// subnormal x itself with underflow, and a range error when it overflows.
static void Test_PrecMathSinh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.108c3aabd6a6p+1, 0x1.108c3aabd6a6p+1, 0x1.108c3aabd6a61p+1, 0x1.108c3aabd6a6p+1};
    static const float valuef[MODES] = {0x1.108c3ap+1f, 0x1.108c3ap+1f, 0x1.108c3cp+1f, 0x1.108c3ap+1f};
    static const long double valuel[MODES] = {0x8.8461d55eb530366p-2L, 0x8.8461d55eb530365p-2L, 0x8.8461d55eb530366p-2L, 0x8.8461d55eb530365p-2L};
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double halfl = 1.5L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_sinh(half), value[i]);
        _TEST_FLOAT(_prec_sinhf(halff), valuef[i]);
        _TEST_FLOAT(_prec_sinhl(halfl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_sinh(0.0), 0.0);
    _TEST_FLOAT(_prec_sinhf(-0.0f), -0.0f);
    _TEST_FLOAT(_prec_sinhl(-inf), -_EXACT_INFINITY);
    _TEST_FLOAT(_prec_sinh(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_sinh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_sinh(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_sinh(-800.0), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathSinh();
    return _Test_Status();
}
