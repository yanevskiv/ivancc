// (Test) Status: 0
// A test file for <float.h> long double characteristics.

// Include helpers.
#include "libc06_00_float_helpers.inc"

// [C99/S5.2.4.2.2p6] long double should have the x87's 80-bit format in #if.
#define DIRECTIVE_LDBL 0
#if (LDBL_MANT_DIG == 64) \
    && (LDBL_DIG == 18) \
    && (LDBL_MIN_EXP == -16381) \
    && (LDBL_MIN_10_EXP == -4931) \
    && (LDBL_MAX_EXP == 16384) \
    && (LDBL_MAX_10_EXP == 4932)
#undef DIRECTIVE_LDBL
#define DIRECTIVE_LDBL 1
#endif

#define BITS_LDBL_MAX     "\377\377\377\377\377\377\377\377\376\177"
#define BITS_LDBL_EPSILON "\000\000\000\000\000\000\000\200\300\077"
#define BITS_LDBL_MIN     "\000\000\000\000\000\000\000\200\001\000"
#define BITS_LDBL_SIZE    10

static const long double extended_max = LDBL_MAX;
static const long double extended_min = LDBL_MIN;
static const long double extended_epsilon = LDBL_EPSILON;

// [C99/S5.2.4.2.2p9] long double should be the x87's extended format, 64 bits.
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

int main(void)
{
    Test_FloatLongDouble();
    return _Test_Status();
}
