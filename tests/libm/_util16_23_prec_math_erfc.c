// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <_prec_math.h> _prec_erfc.

// Include helpers.
#include "_util16_00_prec_math_helpers.inc"

// Defines.
#define MODES 4

// _prec_erfc should give 1 less the error function of x in the current
// direction in each type, 1 of a zero exactly, +0 and 2 of the infinities
// raising nothing, and a range error when its result underflows to zero.
static void Test_PrecMathErfc(void)
{
    static const int modes[MODES] = {_FPU_TONEAREST, _FPU_DOWNWARD, _FPU_UPWARD, _FPU_TOWARDZERO};
    static const double value[MODES] = {0x1.b82879728f11ep-1, 0x1.b82879728f11dp-1, 0x1.b82879728f11ep-1, 0x1.b82879728f11dp-1};
    static const float valuef[MODES] = {0x1.b8287ap-1f, 0x1.b82878p-1f, 0x1.b8287ap-1f, 0x1.b82878p-1f};
    static const long double valuel[MODES] = {0xd.c143cb94788ed18p-4L, 0xd.c143cb94788ed17p-4L, 0xd.c143cb94788ed18p-4L, 0xd.c143cb94788ed17p-4L};
    volatile double eighth = 0.125;
    volatile float eighthf = 0.125f;
    volatile long double eighthl = 0.125L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        _Fpu_WriteRound(modes[i]);
        _Fpu_ClearFlags(_FPU_EXCEPTS);
        _TEST_FLOAT(_prec_erfc(eighth), value[i]);
        _TEST_FLOAT(_prec_erfcf(eighthf), valuef[i]);
        _TEST_FLOAT(_prec_erfcl(eighthl), valuel[i]);
        _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    }
    _Fpu_WriteRound(_FPU_TONEAREST);

    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _TEST_FLOAT(_prec_erfc(0.0), 1.0);
    _TEST_FLOAT(_prec_erfcf(-0.0f), 1.0f);
    _TEST_FLOAT(_prec_erfcl(inf), 0.0L);
    _TEST_FLOAT(_prec_erfc(-inf), 2.0);
    _TEST_TRUE(_Exact_IsNan(_prec_erfc(dnan)));
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), 0);

    _Err_Errno = 0;
    _TEST_FLOAT(_prec_erfc(30.0), 0.0);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_UNDERFLOW | _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
    _Err_Errno = 0;
    _TEST_FLOAT(_prec_erfcl(-30.0L), 2.0L);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Fpu_TestFlags(_FPU_EXCEPTS), _FPU_INEXACT);
    _Fpu_ClearFlags(_FPU_EXCEPTS);
}

int main(void)
{
    Test_PrecMathErfc();
    return _Test_Status();
}
