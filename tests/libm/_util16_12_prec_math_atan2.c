// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_atan2.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_atan2 should give the arc tangent of y/x in [-pi, +pi] by the
// signs of both in each type, in the current direction for float and long
// double, Annex F's results of zeros and infinities, raising nothing of zeros,
// and a range error when the result underflows to zero.
static void Test_PrecMathAtan2(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.4978fap-1f, -0x1.4978fcp-1f, -0x1.4978fap-1f, -0x1.4978fap-1f};
    static const long double valuel[MODES] = {-0x9.6cbe3f9990e91a8p-2L, -0x9.6cbe3f9990e91a8p-2L, -0x9.6cbe3f9990e91a7p-2L, -0x9.6cbe3f9990e91a7p-2L};
    volatile double one = 1.0;
    volatile float threef = -3.0f;
    volatile long double onel = -1.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_atan2f(threef, 4.0f), valuef[i]);
        _TEST_FLOAT(__prec_math_atan2l(onel, onel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_atan2(one, -2.0), 0x1.56c6e7397f5aep+1);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_atan2(0.0, 0.0), 0.0);
    _TEST_FLOAT(__prec_math_atan2(-0.0, 0.0), -0.0);
    _TEST_FLOAT(__prec_math_atan2f(-0.0f, 2.0f), -0.0f);
    _TEST_FLOAT(__prec_math_atan2l(0.0L, inf), 0.0L);
    _TEST_FLOAT(__prec_math_atan2(-1.0, inf), -0.0);
    _TEST_TRUE(_Exact_IsNan(__prec_math_atan2(dnan, 1.0)));
    _TEST_TRUE(_Exact_IsNan(__prec_math_atan2(inf, dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _TEST_FLOAT(__prec_math_atan2(0.0, -0.0), 0x1.921fb54442d18p+1);
    _TEST_FLOAT(__prec_math_atan2(-0.0, -0.0), -0x1.921fb54442d18p+1);
    _TEST_FLOAT(__prec_math_atan2(-0.0, -1.0), -0x1.921fb54442d18p+1);
    _TEST_FLOAT(__prec_math_atan2(1.0, -inf), 0x1.921fb54442d18p+1);
    _TEST_FLOAT(__prec_math_atan2(1.0, 0.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(__prec_math_atan2(-1.0, -0.0), -0x1.921fb54442d18p+0);
    _TEST_FLOAT(__prec_math_atan2(inf, 1.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(__prec_math_atan2(-inf, -inf), -0x1.2d97c7f3321d2p+1);
    _TEST_FLOAT(__prec_math_atan2f((float) inf, (float) inf), 0x1.921fb6p-1f);
    _TEST_FLOAT(__prec_math_atan2l(-inf, inf), -0xc.90fdaa22168c235p-4L);

    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_atan2(0x1p-1000, 0x1p100), 0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAtan2();
    return _Test_Status();
}
