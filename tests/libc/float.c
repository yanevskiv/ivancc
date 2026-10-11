// (Test) Status: 0
// <float.h> defines the characteristics of float, double and long double (S7.7, S5.2.4.2.2), those of x86-64 Linux: a radix of 2, rounding to nearest, no evaluation wider than a type,
// IEC 60559's single and double formats and the x87's extended one for long double, each integer value but FLT_ROUNDS fit for #if and each floating value a constant expression of its type,
// the greatest value, the least normalized one and the step above 1 holding the bits their formats give them, a hexadecimal constant's value, and what the model makes of the radix, the precision and the exponents' range,
// and the decimal digits and exponents following from the binary ones by the logarithm of 2.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>

#if FLT_RADIX == 2 && FLT_EVAL_METHOD == 0 && DECIMAL_DIG == 21
#define DIRECTIVE_MODEL 1
#else
#define DIRECTIVE_MODEL 0
#endif

#if FLT_MANT_DIG == 24 && FLT_DIG == 6 && FLT_MIN_EXP == -125 && FLT_MIN_10_EXP == -37 && FLT_MAX_EXP == 128 && FLT_MAX_10_EXP == 38
#define DIRECTIVE_FLT 1
#else
#define DIRECTIVE_FLT 0
#endif

#if DBL_MANT_DIG == 53 && DBL_DIG == 15 && DBL_MIN_EXP == -1021 && DBL_MIN_10_EXP == -307 && DBL_MAX_EXP == 1024 && DBL_MAX_10_EXP == 308
#define DIRECTIVE_DBL 1
#else
#define DIRECTIVE_DBL 0
#endif

#if LDBL_MANT_DIG == 64 && LDBL_DIG == 18 && LDBL_MIN_EXP == -16381 && LDBL_MIN_10_EXP == -4931 && LDBL_MAX_EXP == 16384 && LDBL_MAX_10_EXP == 4932
#define DIRECTIVE_LDBL 1
#else
#define DIRECTIVE_LDBL 0
#endif

#define BITS_FLT_MAX      "\377\377\177\177"
#define BITS_FLT_EPSILON  "\000\000\000\064"
#define BITS_FLT_MIN      "\000\000\200\000"
#define BITS_DBL_MAX      "\377\377\377\377\377\377\357\177"
#define BITS_DBL_EPSILON  "\000\000\000\000\000\000\260\074"
#define BITS_DBL_MIN      "\000\000\000\000\000\000\020\000"
#define BITS_LDBL_MAX     "\377\377\377\377\377\377\377\377\376\177"
#define BITS_LDBL_EPSILON "\000\000\000\000\000\000\000\200\300\077"
#define BITS_LDBL_MIN     "\000\000\000\000\000\000\000\200\001\000"
#define BITS_LDBL_SIZE    10

#define LOG_NUMERATOR   30103
#define LOG_DENOMINATOR 100000

static const float float_max = FLT_MAX;
static const float float_min = FLT_MIN;
static const float float_epsilon = FLT_EPSILON;
static const double double_max = DBL_MAX;
static const double double_min = DBL_MIN;
static const double double_epsilon = DBL_EPSILON;
static const long double extended_max = LDBL_MAX;
static const long double extended_min = LDBL_MIN;
static const long double extended_epsilon = LDBL_EPSILON;

// Return a value times the radix to a power, which no rounding touches.
static long double scaled(long double value, int power)
{
    while (power > 0) {
        value *= FLT_RADIX;
        power--;
    }
    while (power < 0) {
        value /= FLT_RADIX;
        power++;
    }
    return value;
}

// Return the decimal digits a count of binary ones is worth, rounded to 0.
static int decimal(int binary)
{
    return binary * LOG_NUMERATOR / LOG_DENOMINATOR;
}

// The radix is 2, sums round to nearest, and no operation runs wider.
static void Test_FloatModel(void)
{
    _TEST_INT(FLT_RADIX, 2);
    _TEST_INT(FLT_ROUNDS, 1);
    _TEST_INT(FLT_EVAL_METHOD, 0);
    _TEST_INT(DIRECTIVE_MODEL, 1);
    _TEST_UINT(sizeof(float), 4);
    _TEST_UINT(sizeof(double), 8);
    _TEST_UINT(sizeof(long double), 16);
}

// float is IEC 60559's single format: 24 bits of precision.
static void Test_FloatFloat(void)
{
    _TEST_INT(FLT_MANT_DIG, 24);
    _TEST_INT(FLT_DIG, 6);
    _TEST_INT(FLT_MIN_EXP, -125);
    _TEST_INT(FLT_MIN_10_EXP, -37);
    _TEST_INT(FLT_MAX_EXP, 128);
    _TEST_INT(FLT_MAX_10_EXP, 38);
    _TEST_INT(DIRECTIVE_FLT, 1);

    _TEST_UINT(sizeof(FLT_MAX), sizeof(float));
    _TEST_UINT(sizeof(FLT_MIN), sizeof(float));
    _TEST_UINT(sizeof(FLT_EPSILON), sizeof(float));
    _TEST_MEM(&float_max, BITS_FLT_MAX, sizeof(float));
    _TEST_MEM(&float_min, BITS_FLT_MIN, sizeof(float));
    _TEST_MEM(&float_epsilon, BITS_FLT_EPSILON, sizeof(float));
    _TEST_FLOAT(FLT_MAX, 0x1.FFFFFEp127F);
    _TEST_FLOAT(FLT_MIN, 0x1p-126F);
    _TEST_FLOAT(FLT_EPSILON, 0x1p-23F);
    _TEST_FLOAT(FLT_MAX, (float) scaled(FLT_RADIX - FLT_EPSILON, FLT_MAX_EXP - 1));
    _TEST_FLOAT(FLT_MIN, (float) scaled(1, FLT_MIN_EXP - 1));
    _TEST_FLOAT(FLT_EPSILON, (float) scaled(1, 1 - FLT_MANT_DIG));
}

// double is IEC 60559's double format: 53 bits of precision.
static void Test_FloatDouble(void)
{
    _TEST_INT(DBL_MANT_DIG, 53);
    _TEST_INT(DBL_DIG, 15);
    _TEST_INT(DBL_MIN_EXP, -1021);
    _TEST_INT(DBL_MIN_10_EXP, -307);
    _TEST_INT(DBL_MAX_EXP, 1024);
    _TEST_INT(DBL_MAX_10_EXP, 308);
    _TEST_INT(DIRECTIVE_DBL, 1);

    _TEST_UINT(sizeof(DBL_MAX), sizeof(double));
    _TEST_UINT(sizeof(DBL_MIN), sizeof(double));
    _TEST_UINT(sizeof(DBL_EPSILON), sizeof(double));
    _TEST_MEM(&double_max, BITS_DBL_MAX, sizeof(double));
    _TEST_MEM(&double_min, BITS_DBL_MIN, sizeof(double));
    _TEST_MEM(&double_epsilon, BITS_DBL_EPSILON, sizeof(double));
    _TEST_FLOAT(DBL_MAX, 0x1.FFFFFFFFFFFFFp1023);
    _TEST_FLOAT(DBL_MIN, 0x1p-1022);
    _TEST_FLOAT(DBL_EPSILON, 0x1p-52);
    _TEST_FLOAT(DBL_MAX, (double) scaled(FLT_RADIX - DBL_EPSILON, DBL_MAX_EXP - 1));
    _TEST_FLOAT(DBL_MIN, (double) scaled(1, DBL_MIN_EXP - 1));
    _TEST_FLOAT(DBL_EPSILON, (double) scaled(1, 1 - DBL_MANT_DIG));
}

// long double is the x87's extended format: 64 bits of precision.
static void Test_FloatLongDouble(void)
{
    _TEST_INT(LDBL_MANT_DIG, 64);
    _TEST_INT(LDBL_DIG, 18);
    _TEST_INT(LDBL_MIN_EXP, -16381);
    _TEST_INT(LDBL_MIN_10_EXP, -4931);
    _TEST_INT(LDBL_MAX_EXP, 16384);
    _TEST_INT(LDBL_MAX_10_EXP, 4932);
    _TEST_INT(DIRECTIVE_LDBL, 1);

    _TEST_UINT(sizeof(LDBL_MAX), sizeof(long double));
    _TEST_UINT(sizeof(LDBL_MIN), sizeof(long double));
    _TEST_UINT(sizeof(LDBL_EPSILON), sizeof(long double));
    _TEST_MEM(&extended_max, BITS_LDBL_MAX, BITS_LDBL_SIZE);
    _TEST_MEM(&extended_min, BITS_LDBL_MIN, BITS_LDBL_SIZE);
    _TEST_MEM(&extended_epsilon, BITS_LDBL_EPSILON, BITS_LDBL_SIZE);
    _TEST_FLOAT(LDBL_MAX, 0x1.FFFFFFFFFFFFFFFEp16383L);
    _TEST_FLOAT(LDBL_MIN, 0x1p-16382L);
    _TEST_FLOAT(LDBL_EPSILON, 0x1p-63L);
    _TEST_FLOAT(LDBL_MAX, scaled(FLT_RADIX - LDBL_EPSILON, LDBL_MAX_EXP - 1));
    _TEST_FLOAT(LDBL_MIN, scaled(1, LDBL_MIN_EXP - 1));
    _TEST_FLOAT(LDBL_EPSILON, scaled(1, 1 - LDBL_MANT_DIG));
}

// An epsilon is the step from 1 to the next value, which half of it misses.
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

// The decimal digits and exponents are the binary ones' worth in tens.
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
    Test_FloatModel();
    Test_FloatFloat();
    Test_FloatDouble();
    Test_FloatLongDouble();
    Test_FloatEpsilon();
    Test_FloatDecimal();
    return _Test_Status();
}
