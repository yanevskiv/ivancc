// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_asin.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_asin should give the arc sine of x in [-pi/2, +pi/2] in each type, in
// the current direction for float and long double, keep a zero's sign
// raising nothing, give pi/2 of 1, a tiny x itself, and a NaN with invalid
// and EDOM outside [-1, +1].
static void Test_PrecMathAsin(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.02be9cp-2f, -0x1.02be9ep-2f, -0x1.02be9cp-2f, -0x1.02be9cp-2f};
    static const long double valuel[MODES] = {0xd.91a98ae3406e041p-4L, 0xd.91a98ae3406e04p-4L, 0xd.91a98ae3406e041p-4L, 0xd.91a98ae3406e04p-4L};
    volatile double half = 0.5;
    volatile float quarterf = -0.25f;
    volatile long double threel = 0.75L;
    volatile double zero = 0.0;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_asinf(quarterf), valuef[i]);
        _TEST_FLOAT(_prec_asinl(threel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(_prec_asin(half), 0x1.0c152382d7366p-1);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_asin(0.0), 0.0);
    _TEST_FLOAT(_prec_asinf(-0.0f), -0.0f);
    _TEST_FLOAT(_prec_asinl(-0.0L), -0.0L);
    _TEST_TRUE(_Exact_IsNan(_prec_asin(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);
    _TEST_FLOAT(_prec_asin(1.0), 0x1.921fb54442d18p+0);
    _TEST_FLOAT(_prec_asinf(-1.0f), -0x1.921fb6p+0f);
    _TEST_FLOAT(_prec_asin(0x1p-70), 0x1p-70);

    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(_prec_asin(2.0)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(_prec_asinl(-1.0000001L)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathAsin();
    return _Test_Status();
}
