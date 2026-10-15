// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_acosh.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_acosh should give the nonnegative inverse hyperbolic cosine of x in the
// current direction in each type, +0 of 1 and +inf of +inf raising
// nothing, and a NaN with invalid and EDOM below 1.
static void Test_PrecMathAcosh(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.ecc2caec5160ap-1, 0x1.ecc2caec51609p-1, 0x1.ecc2caec5160ap-1, 0x1.ecc2caec51609p-1};
    static const float valuef[MODES] = {0x1.ecc2cap-1f, 0x1.ecc2cap-1f, 0x1.ecc2ccp-1f, 0x1.ecc2cap-1f};
    static const long double valuel[MODES] = {0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L, 0xb.17217f7d1cf79acp-4L, 0xb.17217f7d1cf79abp-4L};
    volatile double half = 1.5;
    volatile float halff = 1.5f;
    volatile long double quarterl = 1.25L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_acosh(half), value[i]);
        _TEST_FLOAT(_prec_acoshf(halff), valuef[i]);
        _TEST_FLOAT(_prec_acoshl(quarterl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_acosh(1.0), 0.0);
    _TEST_FLOAT(_prec_acoshf(1.0f), 0.0f);
    _TEST_FLOAT(_prec_acoshl(1.0L), 0.0L);
    _TEST_FLOAT(_prec_acosh(inf), inf);
    _TEST_TRUE(_Exact_IsNan(_prec_acosh(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(_prec_acosh(0.5)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(_prec_acoshf((float) -inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAcosh();
    return _Test_Status();
}
