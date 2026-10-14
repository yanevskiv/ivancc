// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> fpclassify.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// fpclassify should class a value by its own type, a long double that is
// normal being subnormal as a double and zero as a float, evaluate its
// argument once, and raise nothing, of a NaN too. The x87's own encodings
// class as glibc's: a set exponent with the integer bit clear a NaN, a zero
// exponent with it set normal.
// [C99:7.12.3.1p2]: fpclassify, by the type of the argument
// [C99:7.12.3.1p3]: fpclassify, the number classification macro of its value
// [C99:7.12.3.1p4]: fpclassify, as ordinary functions chosen by sizeof
// [Amd64-Vol1:6.3.4.2]: the x87's unsupported encodings
static void Test_MathClassify(void)
{
    volatile double zero = 0.0;
    volatile float fzero = 0.0f;
    volatile long double lzero = 0.0L;
    volatile long double tiny = 0x1p-1050L;
    double dnan = zero / zero;
    float fnan = fzero / fzero;
    long double lnan = lzero / lzero;
    int count = 0;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(fpclassify(0.0f), FP_ZERO);
    _TEST_INT(fpclassify(-0.0f), FP_ZERO);
    _TEST_INT(fpclassify(0x1p-149f), FP_SUBNORMAL);
    _TEST_INT(fpclassify(0x1p-126f), FP_NORMAL);
    _TEST_INT(fpclassify(FLT_MAX), FP_NORMAL);
    _TEST_INT(fpclassify(1.0f / fzero), FP_INFINITE);
    _TEST_INT(fpclassify(-1.0f / fzero), FP_INFINITE);
    _TEST_INT(fpclassify(fnan), FP_NAN);
    _TEST_INT(fpclassify(-0.0), FP_ZERO);
    _TEST_INT(fpclassify(0x1p-1074), FP_SUBNORMAL);
    _TEST_INT(fpclassify(0x1p-1022), FP_NORMAL);
    _TEST_INT(fpclassify(0x1p-149), FP_NORMAL);
    _TEST_INT(fpclassify(DBL_MAX), FP_NORMAL);
    _TEST_INT(fpclassify(1.0 / zero), FP_INFINITE);
    _TEST_INT(fpclassify(dnan), FP_NAN);
    _TEST_INT(fpclassify(-0.0L), FP_ZERO);
    _TEST_INT(fpclassify(0x1p-16445L), FP_SUBNORMAL);
    _TEST_INT(fpclassify(0x1p-16382L), FP_NORMAL);
    _TEST_INT(fpclassify(0x1p-1074L), FP_NORMAL);
    _TEST_INT(fpclassify(LDBL_MAX), FP_NORMAL);
    _TEST_INT(fpclassify(1.0L / lzero), FP_INFINITE);
    _TEST_INT(fpclassify(lnan), FP_NAN);
    _TEST_INT(fetestexcept(FE_INVALID), 0);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_INT(fpclassify((double) tiny), FP_SUBNORMAL);
    _TEST_INT(fpclassify((float) tiny), FP_ZERO);
    _TEST_INT(fpclassify(tiny), FP_NORMAL);
    feclearexcept(FE_ALL_EXCEPT);

    _TEST_INT(fpclassify((double) ++count), FP_NORMAL);
    _TEST_INT(count, 1);

    _TEST_INT(fpclassify(ldouble_of(0x7FFF, 0x0000000000000000)), FP_NAN);
    _TEST_INT(fpclassify(ldouble_of(0x7FFF, 0x0000000000000001)), FP_NAN);
    _TEST_INT(fpclassify(ldouble_of(0x3FFF, 0x4000000000000000)), FP_NAN);
    _TEST_INT(fpclassify(ldouble_of(0x0000, 0x8000000000000000)), FP_NORMAL);
    _TEST_INT(fpclassify(ldouble_of(0x7FFF, 0x8000000000000000)), FP_INFINITE);
    _TEST_INT(fpclassify(ldouble_of(0x7FFF, 0xC000000000000000)), FP_NAN);
}

int main(void)
{
    Test_MathClassify();
    return _Test_Status();
}
