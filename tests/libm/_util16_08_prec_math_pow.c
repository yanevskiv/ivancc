// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> __prec_math_pow.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// __prec_math_pow should give x to y in the current direction in each type, an
// integer power's value, Annex F's results of a zero, 1, -1, an infinity
// and a NaN raising nothing, a pole with divide-by-zero and ERANGE, a NaN
// with invalid and EDOM of a negative x and a non-integer y, and a range
// error when the result overflows to an infinity or underflows to zero.
static void Test_PrecMathPow(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.63f08b6f382aep+0, 0x1.63f08b6f382aep+0, 0x1.63f08b6f382afp+0, 0x1.63f08b6f382aep+0};
    static const float valuef[MODES] = {0x1.63f08cp+0f, 0x1.63f08ap+0f, 0x1.63f08cp+0f, 0x1.63f08ap+0f};
    static const long double valuel[MODES] = {0xb.1f845b79c15733cp-3L, 0xb.1f845b79c15733bp-3L, 0xb.1f845b79c15733cp-3L, 0xb.1f845b79c15733bp-3L};
    volatile double three = 3.0;
    volatile float threef = 3.0f;
    volatile long double threel = 3.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(__prec_math_pow(three, 0.3), value[i]);
        _TEST_FLOAT(__prec_math_powf(threef, 0.3f), valuef[i]);
        _TEST_FLOAT(__prec_math_powl(threel, 0.3L), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _TEST_FLOAT(__prec_math_pow(three, 2.0), 9.0);
    _TEST_FLOAT(__prec_math_pow(-three, 3.0), -27.0);
    _TEST_FLOAT(__prec_math_powf(4.0f, 0.5f), 2.0f);
    _TEST_FLOAT(__prec_math_powl(2.0L, -16445.0L), 0x1p-16445L);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(__prec_math_pow(dnan, -0.0), 1.0);
    _TEST_FLOAT(__prec_math_pow(1.0, dnan), 1.0);
    _TEST_FLOAT(__prec_math_pow(-1.0, inf), 1.0);
    _TEST_FLOAT(__prec_math_powf(-1.0f, -_EXACT_INFINITY), 1.0f);
    _TEST_FLOAT(__prec_math_pow(-zero, 3.0), -0.0);
    _TEST_FLOAT(__prec_math_pow(-zero, 0.5), 0.0);
    _TEST_FLOAT(__prec_math_pow(0.5, inf), 0.0);
    _TEST_FLOAT(__prec_math_pow(-0.5, -inf), inf);
    _TEST_FLOAT(__prec_math_pow(2.0, -inf), 0.0);
    _TEST_FLOAT(__prec_math_powl(-2.0L, inf), inf);
    _TEST_FLOAT(__prec_math_pow(-inf, -3.0), -0.0);
    _TEST_FLOAT(__prec_math_pow(-inf, -2.0), 0.0);
    _TEST_FLOAT(__prec_math_pow(-inf, 3.0), -inf);
    _TEST_FLOAT(__prec_math_pow(-inf, 0.5), inf);
    _TEST_FLOAT(__prec_math_pow(inf, -1.0), 0.0);
    _TEST_FLOAT(__prec_math_powl(inf, 0.5L), inf);
    _TEST_TRUE(_Exact_IsNan(__prec_math_pow(dnan, 1.0)));
    _TEST_TRUE(_Exact_IsNan(__prec_math_pow(2.0, dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_pow(zero, -1.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_DIVBYZERO);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_pow(-zero, -3.0), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_powf(-0.0f, -2.0f), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_TRUE(_Exact_IsNan(__prec_math_pow(-8.0, 1.0 / three)));
    _TEST_INT(_Err_Errno, _SYS_EDOM);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS) & ~_FPU_INEXACT, _FPU_INVALID);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;

    _TEST_FLOAT(__prec_math_pow(2.0, 1024.0), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_OVERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_pow(-10.0, 309.0), -_EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_powl(2.0L, 16384.0L), _EXACT_INFINITY);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_pow(2.0, -1080.0), 0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(__prec_math_pow(10.0, -310.0), 0x0.012688b70e62bp-1022);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathPow();
    return _Test_Status();
}
