// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> lrint, llrint, lround and llround.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// lrint and llrint should round to an integer type in the current direction
// and lround and llround halfway away from zero, from each type, raising
// invalid out of range and lrint inexact when the value changes.
// [C99:7.12.9.5p2]: lrint and llrint, in the current direction
// [C99:7.12.9.7p2]: lround and llround, halfway away from zero
// [C99:F.9.6.5p1]:  lrint and llrint, invalid out of range, else inexact
// [C99:F.9.6.7p1]:  lround and llround, as lrint by default but halfway
static void Test_MathLrint(void)
{
    volatile double zero = 0.0;
    double inf = 1.0 / zero;
    double dnan = zero / zero;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(lrint(-3.0), -3);
    _TEST_INT(llrint(0x1p62), 0x4000000000000000LL);
    _TEST_INT(lrintl(-0x1p63L), LONG_MIN);
    _TEST_INT(llrintl(0x1.fffffffffffffffcp62L), 0x7FFFFFFFFFFFFFFFLL);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_INT(lrint(2.5), 2);
    _TEST_INT(fetestexcept(FE_INEXACT), FE_INEXACT);
    _TEST_INT(lrintf(-2.5f), -2);
    _TEST_INT(llrintl(-4611686018427387904.5L), -0x4000000000000000LL);
    fesetround(FE_UPWARD);
    _TEST_INT(lrint(2.1), 3);
    _TEST_INT(llrintf(-2.9f), -2);
    fesetround(FE_TOWARDZERO);
    _TEST_INT(lrintl(-2.9L), -2);
    fesetround(FE_TONEAREST);
    _TEST_INT(fetestexcept(FE_INVALID), 0);

    _TEST_INT(lround(2.5), 3);
    _TEST_INT(lround(-2.5), -3);
    _TEST_INT(lround(0x1.fffffffffffffp-2), 0);
    _TEST_INT(lroundf(-0.5f), -1);
    _TEST_INT(llround(-0x1p63), LLONG_MIN);
    _TEST_INT(llroundl(0x1.fffffffffffffffcp62L), 0x7FFFFFFFFFFFFFFFLL);
    _TEST_INT(lroundl(-2305843009213693952.5L), -2305843009213693953L);
    fesetround(FE_DOWNWARD);
    _TEST_INT(lround(2.5), 3);
    fesetround(FE_TONEAREST);
    _TEST_INT(fetestexcept(FE_INVALID), 0);

    lrint(0x1p63);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    llrintf((float) dnan);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    lround(-inf);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    llroundl(0x1p63L);
    _TEST_INT(fetestexcept(FE_INVALID), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_MathLrint();
    return _Test_Status();
}
