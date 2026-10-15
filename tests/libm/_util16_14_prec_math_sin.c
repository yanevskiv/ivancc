// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_sin.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_sin should give the sine of x in radians in each type, in the
// current direction for float and long double, of the largest long doubles too,
// keep a zero's sign raising nothing, give a subnormal x itself with underflow,
// and a NaN with invalid and EDOM of an infinity.
static void Test_PrecMathSin(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const float valuef[MODES] = {0x1.d18f6ep-1f, 0x1.d18f6ep-1f, 0x1.d18f7p-1f, 0x1.d18f6ep-1f};
    static const long double valuel[MODES] = {0xb.301c733f168e24ep-4L, 0xb.301c733f168e24dp-4L, 0xb.301c733f168e24ep-4L, 0xb.301c733f168e24dp-4L};
    volatile double half = 0.5;
    volatile float twof = 2.0f;
    volatile long double hugel = 0x1p16000L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_sinf(twof), valuef[i]);
        _TEST_FLOAT(__prec_math_sinl(hugel), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);
    _TEST_FLOAT(__prec_math_sin(half), 0x1.eaee8744b05fp-2);
    _TEST_FLOAT(__prec_math_sin(1e300), -0x1.a2c16b010e385p-1);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_sin(0.0), 0.0);
    _TEST_FLOAT(__prec_math_sinf(-0.0f), -0.0f);
    _TEST_FLOAT(__prec_math_sinl(-0.0L), -0.0L);
    _TEST_TRUE(_Exact_IsNan(__prec_math_sin(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_sin(0x1p-1074), 0x1p-1074);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_TRUE(_Exact_IsNan(__prec_math_sin(inf)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INVALID);
    _Err_Errno = 0;
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathSin();
    return _Test_Status();
}
