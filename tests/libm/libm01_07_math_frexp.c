// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> frexp.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// frexp should break a value into a fraction in [1/2, 1) and a power of 2
// in each type, a subnormal's as if normalized, keep a zero, an infinity
// and a NaN, and raise nothing.
// [C99:7.12.6.4p3]: frexp, a fraction in [1/2, 1) or zero, value its x 2^*exp
// [C99:F.9.3.4p1]:  frexp of a zero, 0 stored, of an infinity and a NaN
// [C99:F.9.3.4p2]:  frexp, raising nothing
static void Test_MathFrexp(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;
    int power = 1;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(frexp(8.0, &power), 0.5);
    _TEST_INT(power, 4);
    _TEST_FLOAT(frexp(-3.0, &power), -0.75);
    _TEST_INT(power, 2);
    _TEST_FLOAT(frexp(0x1p-1074, &power), 0.5);
    _TEST_INT(power, -1073);
    _TEST_FLOAT(frexp(DBL_MAX, &power), 0x1.fffffffffffffp-1);
    _TEST_INT(power, 1024);
    _TEST_FLOAT(frexp(-0.0, &power), -0.0);
    _TEST_INT(power, 0);
    _TEST_FLOAT(frexp(-inf, &power), -inf);
    _TEST_TRUE(isnan(frexp(dnan, &power)));

    _TEST_FLOAT(frexpf(0x1p-149f, &power), 0.5f);
    _TEST_INT(power, -148);
    _TEST_FLOAT(frexpf(-FLT_MAX, &power), -0x1.fffffep-1f);
    _TEST_INT(power, 128);
    _TEST_FLOAT(frexpf(0.0f, &power), 0.0f);
    _TEST_INT(power, 0);

    _TEST_FLOAT(frexpl(0x1p-16445L, &power), 0.5L);
    _TEST_INT(power, -16444);
    _TEST_FLOAT(frexpl(LDBL_MAX, &power), 0x1.fffffffffffffffep-1L);
    _TEST_INT(power, 16384);
    _TEST_FLOAT(frexpl(-0x1.8p-16400L, &power), -0.75L);
    _TEST_INT(power, -16399);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathFrexp();
    return _Test_Status();
}
