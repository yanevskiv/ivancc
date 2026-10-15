// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_tan.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_tan should give the tangent of x in radians in each type, in the
// current direction for float and long double, keep a zero's sign raising
// nothing, and give a NaN with invalid and EDOM of an infinity.
static void Test_PrecMathTan(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {-0x1.17af62p+1f, -0x1.17af64p+1f, -0x1.17af62p+1f, -0x1.17af62p+1f};
    static const long double valuel[MODES] = {0x9.1f7b892a5c37866p-6L, 0x9.1f7b892a5c37866p-6L, 0x9.1f7b892a5c37867p-6L, 0x9.1f7b892a5c37866p-6L};
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double threel = -3.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_tanf(twof), valuef[i]);
        _TEST_FLOAT(__prec_math_tanl(threel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_tan(half), 0x1.17b4f5bf3474ap-1);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_tan(0.0), 0.0);
    _TEST_FLOAT(__prec_math_tanf(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_tanl(-0.0L), -0.0L);
    _TEST_TRUE(_Exact_IsNan(__prec_math_tan(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_tan(inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathTan();
    return _Test_Status();
}
