// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> nextafter and nexttoward.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// nextafter and nexttoward should step x to the next value of its type
// toward y, give y when x equals it, cross zero and the subnormals, raise
// overflow and inexact for an infinite result of a finite x, and underflow
// and inexact for a subnormal or zero result.
// [C99:7.12.11.3p2]: nextafter, the next value after x toward y, y if equal
// [C99:7.12.11.4p2]: nexttoward, nextafter toward a long double y, its footnote 212
// [C99:F.9.8.3p1]:   nextafter, overflow or underflow, and inexact
static void Test_MathNextafter(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafter(1.0, 2.0), 0x1.0000000000001p0);
    _TEST_FLOAT(nextafter(1.0, -inf), 0x1.fffffffffffffp-1);
    _TEST_FLOAT(nextafter(-1.0, 0.0), -0x1.fffffffffffffp-1);
    _TEST_FLOAT(nextafter(0x1.ffffffffffffep-1023, 1.0), DBL_MIN);
    _TEST_FLOAT(nextafter(inf, 0.0), DBL_MAX);
    _TEST_FLOAT(nextafter(-inf, 0.0), -DBL_MAX);
    _TEST_FLOAT(nextafter(0.0, -0.0), -0.0);
    _TEST_FLOAT(nextafter(-0.0, 0.0), 0.0);
    _TEST_FLOAT(nextafter(2.0, 2.0), 2.0);
    _TEST_TRUE(isnan(nextafter(1.0, dnan)));
    _TEST_TRUE(isnan(nextafter(dnan, 1.0)));
    _TEST_FLOAT(nextafterf(1.0f, 2.0f), 0x1.000002p0f);
    _TEST_FLOAT(nextafterf(-FLT_MAX, (float) -inf), (float) -inf);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafterl(1.0L, 2.0L), 0x1.0000000000000002p0L);
    _TEST_FLOAT(nextafterl(1.0L, 0.0L), 0x1.fffffffffffffffep-1L);
    _TEST_FLOAT(nextafterl(0x1.fffffffffffffffcp-16383L, 1.0L), LDBL_MIN);
    _TEST_FLOAT(nextafterl(-LDBL_MAX, 0.0L), -0x1.fffffffffffffffcp16383L);
    _TEST_FLOAT(nexttoward(1.0, 1.0L + 0x1p-60L), 0x1.0000000000001p0);
    _TEST_FLOAT(nexttoward(1.0, 1.0L - 0x1p-60L), 0x1.fffffffffffffp-1);
    _TEST_FLOAT(nexttowardf(-1.0f, -1.0L), -1.0f);
    _TEST_FLOAT(nexttowardl(-0.0L, 0.0L), 0.0L);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);

    _TEST_FLOAT(nextafter(DBL_MAX, inf), inf);
    _TEST_INT(fetestexcept(FE_OVERFLOW | FE_INEXACT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafterl(LDBL_MAX, (long double) inf), (long double) inf);
    _TEST_INT(fetestexcept(FE_OVERFLOW | FE_INEXACT), FE_OVERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafter(0.0, -1.0), -0x1p-1074);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafter(DBL_MIN, 0.0), 0x1.ffffffffffffep-1023);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafterf(-0x1p-149f, 1.0f), -0.0f);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nextafterl(LDBL_MIN, 0.0L), 0x1.fffffffffffffffcp-16383L);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nexttoward(0x1p-1074, 0x1p-1080L), 0.0);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(nexttowardl(0.0L, -1.0L), -0x1p-16445L);
    _TEST_INT(fetestexcept(FE_UNDERFLOW | FE_INEXACT), FE_UNDERFLOW | FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathNextafter();
    return _Test_Status();
}
