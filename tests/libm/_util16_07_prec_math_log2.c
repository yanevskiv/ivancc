// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_log2.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_log2 should give the base-2 logarithm in the current direction in each
// type, a power of 2's exponent, +0 of 1 in every direction, -inf with
// divide-by-zero and ERANGE of a zero, and a NaN with invalid and EDOM
// below zero.
static void Test_PrecMathLog2(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.95c01a39fbd68p+0, 0x1.95c01a39fbd68p+0, 0x1.95c01a39fbd69p+0, 0x1.95c01a39fbd68p+0};
    static const float valuef[MODES] = {0x1.95c01ap+0f, 0x1.95c01ap+0f, 0x1.95c01cp+0f, 0x1.95c01ap+0f};
    static const long double valuel[MODES] = {0xc.ae00d1cfdeb43dp-3L, 0xc.ae00d1cfdeb43cfp-3L, 0xc.ae00d1cfdeb43dp-3L, 0xc.ae00d1cfdeb43cfp-3L};
    volatile double three = 3.0;
    volatile float threef = 3.0f;
    volatile long double threel = 3.0L;
    volatile double one = 1.0;
    volatile double eight = 8.0;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_log2(three), value[i]);
        _TEST_FLOAT(_prec_log2f(threef), valuef[i]);
        _TEST_FLOAT(_prec_log2l(threel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_log2(one), 0.0);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _TEST_FLOAT(_prec_log2(eight), 3.0);
    _TEST_FLOAT(_prec_log2f(0x1p-149f), -149.0f);
    _TEST_FLOAT(_prec_log2l(0x1p-16445L), -16445.0L);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_log2(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_log2(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_log2(zero), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(_prec_log2l(-2.0L)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
}

int main(void)
{
    Test_PrecMathLog2();
    return _Test_Status();
}
