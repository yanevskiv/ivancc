// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> ceil, floor, round and trunc.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// ceil, floor, round and trunc should round to an integer in each type,
// whatever the current direction, keep a zero's sign and an infinity, and
// raise no invalid.
// [C99:7.12.9.1p2]: ceil, the least integer not below x
// [C99:7.12.9.2p2]: floor, the greatest integer not above x
// [C99:7.12.9.6p2]: round, halfway cases away from zero, whatever the direction
// [C99:7.12.9.8p2]: trunc, toward zero
// [C99:F.9.6.1p1]:  ceil of a zero and an infinity
// [C99:F.9.6.2p1]:  floor of a zero and an infinity
// [C99:F.9.6.6p1]:  round of a zero and an infinity
// [C99:F.9.6.8p1]:  trunc of a zero and an infinity, whatever the direction
static void Test_MathRounding(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_FLOAT(ceil(2.5), 3.0);
    _TEST_FLOAT(ceil(-2.5), -2.0);
    _TEST_FLOAT(ceil(-0.5), -0.0);
    _TEST_FLOAT(ceil(0x1p-1074), 1.0);
    _TEST_FLOAT(ceil(0x1.0000000000001p51), 0x1.0000000000002p51);
    _TEST_FLOAT(ceil(-0.0), -0.0);
    _TEST_FLOAT(ceil(-inf), -inf);
    _TEST_FLOAT(floor(2.5), 2.0);
    _TEST_FLOAT(floor(-2.5), -3.0);
    _TEST_FLOAT(floor(0.5), 0.0);
    _TEST_FLOAT(floor(-0x1p-1074), -1.0);
    _TEST_FLOAT(floor(DBL_MAX), DBL_MAX);
    _TEST_FLOAT(floor(-0.0), -0.0);
    _TEST_FLOAT(round(2.5), 3.0);
    _TEST_FLOAT(round(-2.5), -3.0);
    _TEST_FLOAT(round(0x1.fffffffffffffp-2), 0.0);
    _TEST_FLOAT(round(-0.5), -1.0);
    _TEST_FLOAT(round(-0.25), -0.0);
    _TEST_FLOAT(round(0x1.0000000000001p51), 0x1.0000000000002p51);
    _TEST_FLOAT(round(inf), inf);
    _TEST_FLOAT(trunc(2.75), 2.0);
    _TEST_FLOAT(trunc(-0.75), -0.0);
    _TEST_FLOAT(trunc(-0x1.fffffffffffffp51), -0x1.ffffffffffffep51);
    _TEST_FLOAT(trunc(-inf), -inf);
    _TEST_TRUE(isnan(ceil(dnan)));
    _TEST_TRUE(isnan(floor(dnan)));
    _TEST_TRUE(isnan(round(dnan)));
    _TEST_TRUE(isnan(trunc(dnan)));

    _TEST_FLOAT(ceilf(-0x1.fffffep-1f), -0.0f);
    _TEST_FLOAT(floorf(0x1.fffffep22f), 8388607.0f);
    _TEST_FLOAT(roundf(0x1.fffffep-2f), 0.0f);
    _TEST_FLOAT(roundf(-1.5f), -2.0f);
    _TEST_FLOAT(truncf(-FLT_MAX), -FLT_MAX);

    _TEST_FLOAT(ceill(2305843009213693952.25L), 2305843009213693953.0L);
    _TEST_FLOAT(floorl(-2305843009213693952.25L), -2305843009213693953.0L);
    _TEST_FLOAT(roundl(0x1.fffffffffffffffep-2L), 0.0L);
    _TEST_FLOAT(roundl(-2305843009213693952.5L), -2305843009213693953.0L);
    _TEST_FLOAT(truncl(-0x1.fffffffffffffffep62L), -0x1.fffffffffffffffcp62L);
    _TEST_FLOAT(truncl(0x1p-16445L), 0.0L);
    _TEST_INT(fetestexcept(FE_INVALID), 0);

    fesetround(FE_UPWARD);
    _TEST_FLOAT(floor(2.5), 2.0);
    _TEST_FLOAT(round(-2.5), -3.0);
    _TEST_FLOAT(trunc(-2.5), -2.0);
    fesetround(FE_DOWNWARD);
    _TEST_FLOAT(ceil(-2.5), -2.0);
    _TEST_FLOAT(round(2.5), 3.0);
    _TEST_FLOAT(round(0.25), 0.0);
    _TEST_FLOAT(trunc(2.5), 2.0);
    fesetround(FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathRounding();
    return _Test_Status();
}
