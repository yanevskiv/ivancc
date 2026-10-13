// (Test) Status: 0
// A test file for <float.h> decimal digits and exponents.

// Include helpers.
#include "libc06_00_float_helpers.inc"

#define LOG_NUMERATOR   30103
#define LOG_DENOMINATOR 100000

// Return the decimal digits a count of binary ones is worth, rounded to 0.
static int decimal(int binary)
{
    return binary * LOG_NUMERATOR / LOG_DENOMINATOR;
}

// [C99/S5.2.4.2.2p9] The decimal digits and exponents should follow the binary.
static void Test_FloatDecimal(void)
{
    _TEST_INT(FLT_DIG, decimal(FLT_MANT_DIG - 1));
    _TEST_INT(DBL_DIG, decimal(DBL_MANT_DIG - 1));
    _TEST_INT(LDBL_DIG, decimal(LDBL_MANT_DIG - 1));
    _TEST_INT(DECIMAL_DIG, decimal(LDBL_MANT_DIG) + 2);
    _TEST_INT(FLT_MIN_10_EXP, decimal(FLT_MIN_EXP - 1));
    _TEST_INT(DBL_MIN_10_EXP, decimal(DBL_MIN_EXP - 1));
    _TEST_INT(LDBL_MIN_10_EXP, decimal(LDBL_MIN_EXP - 1));
    _TEST_INT(FLT_MAX_10_EXP, decimal(FLT_MAX_EXP));
    _TEST_INT(DBL_MAX_10_EXP, decimal(DBL_MAX_EXP));
    _TEST_INT(LDBL_MAX_10_EXP, decimal(LDBL_MAX_EXP));
}

int main(void)
{
    Test_FloatDecimal();
    return _Test_Status();
}
