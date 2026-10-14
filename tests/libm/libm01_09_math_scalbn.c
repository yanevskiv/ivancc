// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> ldexp, scalbn and scalbln.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// ldexp, scalbn and scalbln should scale by a power of 2 in each type,
// rounding once in the current direction, overflow to HUGE_VAL with ERANGE
// and overflow, underflow with underflow, and keep a zero and an infinity.
// [C99:7.12.6.6p3]:   ldexp, x times 2 to exp
// [C99:7.12.6.13p3]:  scalbn and scalbln, x times FLT_RADIX to n
// [C99:7.12.1p4]:     an overflow, HUGE_VAL, ERANGE and overflow
// [C99:F.9p6]:        overflow, an infinity or the greatest finite value
// [C99:F.9p7]:        underflow, a tiny result inexact
// [C99:F.9.3.6p1]:    ldexp, scalbn on a binary system
// [C99:F.9.3.13p1]:   scalbn of a zero, an infinity, and by 0
static void Test_MathScalbn(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(ldexp(1.0, 10), 1024.0);
    _TEST_FLOAT(ldexp(0x1p-1074, 1074), 1.0);
    _TEST_FLOAT(scalbn(1.0, -1074), 0x1p-1074);
    _TEST_FLOAT(scalbn(-0x1.8p1, 0), -0x1.8p1);
    _TEST_FLOAT(scalbn(-0.0, 5), -0.0);
    _TEST_FLOAT(scalbn(-inf, -5), -inf);
    _TEST_FLOAT(scalbnf(0x1p-149f, 276), 0x1p127f);
    _TEST_FLOAT(scalbnl(LDBL_MIN, -63), 0x1p-16445L);
    _TEST_FLOAT(scalblnl(0x1p-16445L, 32766L), 0x1p16321L);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_FLOAT(scalbn(3.0, -1075), 0x1p-1073);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(scalblnl(1.0L, -16446L), 0.0L);
    _TEST_INT(fetestexcept(FE_UNDERFLOW), FE_UNDERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(scalbln(-1.0, LONG_MIN), -0.0);
    feclearexcept(FE_ALL_EXCEPT);

    errno = 0;
    _TEST_FLOAT(ldexpf(1.0f, 128), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_OVERFLOW), FE_OVERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(scalbln(-1.0, LONG_MAX), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(ldexpl(LDBL_MAX, 1), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;

    fesetround(FE_UPWARD);
    _TEST_FLOAT(scalbn(1.0, -2000), 0x1p-1074);
    fesetround(FE_TOWARDZERO);
    _TEST_FLOAT(scalbnf(1.0f, 2000), FLT_MAX);
    _TEST_INT(fetestexcept(FE_OVERFLOW), FE_OVERFLOW);
    fesetround(FE_DOWNWARD);
    _TEST_FLOAT(scalbnl(-3.0L, -16447), -0x1p-16445L);
    fesetround(FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathScalbn();
    return _Test_Status();
}
