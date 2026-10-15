// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> pow.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define MODES 4
#define ULPS  4

// pow should give x to y within a few ulps in each direction and type, an
// integer power's value, Annex F's results of a zero, 1, -1, an infinity
// and a NaN raising nothing, a pole with divide-by-zero and ERANGE, a NaN
// with invalid and EDOM of a negative x and a non-integer y, and a range
// error when the result overflows to an infinity or underflows to zero.
// [C99:7.12.7.4p2]:  pow, a domain error of a negative x and a non-integer y
// [C99:7.12.1p2]:    a domain error, EDOM and invalid
// [C99:7.12.1p4]:    an overflow or an exact infinity, HUGE_VAL and ERANGE
// [C99:7.12.1p5]:    an underflow, ERANGE implementation-defined
// [C99:F.9.4.4p1]:   pow of a zero, 1, -1, an infinity and a NaN
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the rounding direction, implementation-defined
static void Test_MathPow(void)
{
    static const int modes[MODES] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    static const double value = 0x1.63f08b6f382aep+0;
    static const float valuef = 0x1.63f08cp+0f;
    static const long double valuel = 0xb.1f845b79c15733cp-3L;
    volatile double three = 3.0;
    volatile float threef = 3.0f;
    volatile long double threel = 3.0L;
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    for (int i = 0; i < MODES; i++) {
        fesetround(modes[i]);
        feclearexcept(FE_ALL_EXCEPT);
        _TEST_TRUE(within_ulps(pow(three, 0.3), value, DBL_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(powf(threef, 0.3f), valuef, FLT_MANT_DIG, ULPS));
        _TEST_TRUE(within_ulps(powl(threel, 0.3L), valuel, LDBL_MANT_DIG, ULPS));
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    }
    fesetround(FE_TONEAREST);

    _TEST_FLOAT(pow(three, 2.0), 9.0);
    _TEST_FLOAT(pow(-three, 3.0), -27.0);
    _TEST_FLOAT(powf(4.0f, 0.5f), 2.0f);
    _TEST_FLOAT(powl(2.0L, -16445.0L), 0x1p-16445L);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(pow(dnan, -0.0), 1.0);
    _TEST_FLOAT(pow(1.0, dnan), 1.0);
    _TEST_FLOAT(pow(-1.0, inf), 1.0);
    _TEST_FLOAT(powf(-1.0f, -HUGE_VALF), 1.0f);
    _TEST_FLOAT(pow(-zero, 3.0), -0.0);
    _TEST_FLOAT(pow(-zero, 0.5), 0.0);
    _TEST_FLOAT(pow(0.5, inf), 0.0);
    _TEST_FLOAT(pow(-0.5, -inf), inf);
    _TEST_FLOAT(pow(2.0, -inf), 0.0);
    _TEST_FLOAT(powl(-2.0L, inf), inf);
    _TEST_FLOAT(pow(-inf, -3.0), -0.0);
    _TEST_FLOAT(pow(-inf, -2.0), 0.0);
    _TEST_FLOAT(pow(-inf, 3.0), -inf);
    _TEST_FLOAT(pow(-inf, 0.5), inf);
    _TEST_FLOAT(pow(inf, -1.0), 0.0);
    _TEST_FLOAT(powl(inf, 0.5L), inf);
    _TEST_TRUE(isnan(pow(dnan, 1.0)));
    _TEST_TRUE(isnan(pow(2.0, dnan)));
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    errno = 0;
    _TEST_FLOAT(pow(zero, -1.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(pow(-zero, -3.0), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(powf(-0.0f, -2.0f), HUGE_VALF);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_TRUE(isnan(pow(-8.0, 1.0 / three)));
    _TEST_INT(errno, EDOM);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT) & ~FE_INEXACT, FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;

    _TEST_FLOAT(pow(2.0, 1024.0), HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(pow(-10.0, 309.0), -HUGE_VAL);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_FLOAT(powl(2.0L, 16384.0L), HUGE_VALL);
    _TEST_INT(errno, ERANGE);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(pow(2.0, -1080.0), 0.0);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    _TEST_FLOAT(pow(10.0, -310.0), 0x0.012688b70e62bp-1022);
    _TEST_INT(errno, 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathPow();
    return _Test_Status();
}
