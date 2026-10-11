// (Test) Status: 0
// A test file for <float.h> epsilons.

// Include helpers.
#include "libc06_00_float_helpers.inc"

// (S5.2.4.2.2p11) An epsilon should be the step from 1 to the next value.
static void Test_FloatEpsilon(void)
{
    float one = 1;
    float above = one + FLT_EPSILON;
    float short_of = one + FLT_EPSILON / FLT_RADIX;
    double wide = 1;
    double wide_above = wide + DBL_EPSILON;
    double wide_short_of = wide + DBL_EPSILON / FLT_RADIX;
    long double extended = 1;
    long double extended_above = extended + LDBL_EPSILON;
    long double extended_short_of = extended + LDBL_EPSILON / FLT_RADIX;

    _TEST_TRUE(above > one);
    _TEST_FLOAT(short_of, one);
    _TEST_INT((long) (one / FLT_EPSILON), 1L << (FLT_MANT_DIG - 1));
    _TEST_TRUE(wide_above > wide);
    _TEST_FLOAT(wide_short_of, wide);
    _TEST_INT((long) (wide / DBL_EPSILON), 1L << (DBL_MANT_DIG - 1));
    _TEST_TRUE(extended_above > extended);
    _TEST_FLOAT(extended_short_of, extended);
    _TEST_UINT((unsigned long) (extended / LDBL_EPSILON), 1UL << (LDBL_MANT_DIG - 1));
}

int main(void)
{
    Test_FloatEpsilon();
    return _Test_Status();
}
