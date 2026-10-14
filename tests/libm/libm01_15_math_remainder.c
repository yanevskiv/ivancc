// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> remainder and remquo.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// remainder and remquo should give x - ny exactly in each type, n nearest
// x/y and even when halfway, a zero with x's sign, remquo the quotient's
// sign and its magnitude modulo 8 at least, and give a NaN and invalid for
// an infinite x or a zero y.
// [C99:7.12.10.2p2]: remainder, IEC 60559's x REM y, its footnote 210
// [C99:7.12.10.3p2]: remquo, the quotient's sign, its magnitude modulo 2^n, n >= 3
// [C99:F.9.7.2p1]:   remainder, as IEC 60559's
// [C99:F.9.7.3p1]:   remquo, as remainder
static void Test_MathRemainder(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;
    int quo = 0;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(remainder(5.0, 2.0), 1.0);
    _TEST_FLOAT(remainder(7.0, 2.0), -1.0);
    _TEST_FLOAT(remainder(3.0, 2.0), -1.0);
    _TEST_FLOAT(remainder(-1.0, 2.0), -1.0);
    _TEST_FLOAT(remainder(-6.0, 3.0), -0.0);
    _TEST_FLOAT(remainder(6.0, -3.0), 0.0);
    _TEST_FLOAT(remainder(DBL_MAX, 3.0), -1.0);
    _TEST_FLOAT(remainder(0x1.8p-1073, 0x1p-1073), -0x1p-1074);
    _TEST_FLOAT(remainder(1.0, inf), 1.0);
    _TEST_FLOAT(remainder(-0.0, 1.0), -0.0);
    _TEST_FLOAT(remainderf(1.5f, 1.0f), -0.5f);
    _TEST_FLOAT(remainderf(-0x1.8p-148f, 0x1p-148f), 0x1p-149f);
    _TEST_FLOAT(remainderl(2305843009213693953.0L, 2.0L), 1.0L);
    _TEST_FLOAT(remainderl(0x1p16383L, 0x1.8p-16440L), 0x1p-16441L);
    _TEST_TRUE(isnan(remainder(1.0, dnan)));

    _TEST_FLOAT(remquo(29.0, 3.0, &quo), -1.0);
    _TEST_INT(quo > 0 && quo % 8 == 2, 1);
    _TEST_FLOAT(remquo(-29.0, 3.0, &quo), 1.0);
    _TEST_INT(quo < 0 && -quo % 8 == 2, 1);
    _TEST_FLOAT(remquo(0x1p60, -3.0, &quo), 1.0);
    _TEST_INT(quo < 0 && -quo % 8 == 5, 1);
    _TEST_FLOAT(remquof(-2.0f, -4.0f, &quo), -2.0f);
    _TEST_INT(quo % 8, 0);
    _TEST_FLOAT(remquol(-1.5L, 1.0L, &quo), 0.5L);
    _TEST_INT(quo < 0 && -quo % 8 == 2, 1);
    _TEST_FLOAT(remquol(11.0L, 2.0L, &quo), -1.0L);
    _TEST_INT(quo > 0 && quo % 8 == 6, 1);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT & ~FE_INEXACT), 0);

    _TEST_TRUE(isnan(remainder(-inf, 1.0)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(remainderl(1.0L, 0.0L)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_TRUE(isnan(remquof(1.0f, -0.0f, &quo)));
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathRemainder();
    return _Test_Status();
}
