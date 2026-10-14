// (Test) Status: 0
// A test file for <float.h> double characteristics.

// Include helpers.
#include "libc06_00_float_helpers.inc"

// double should have IEC 60559's double format in #if.
// [C99:5.2.4.2.2p6]: <float.h>'s values, constant expressions, most fit for #if
#define DIRECTIVE_DBL 0
#if (DBL_MANT_DIG == 53) \
    && (DBL_DIG == 15) \
    && (DBL_MIN_EXP == -1021) \
    && (DBL_MIN_10_EXP == -307) \
    && (DBL_MAX_EXP == 1024) \
    && (DBL_MAX_10_EXP == 308)
#undef DIRECTIVE_DBL
#define DIRECTIVE_DBL 1
#endif

#define BITS_DBL_MAX     "\377\377\377\377\377\377\357\177"
#define BITS_DBL_EPSILON "\000\000\000\000\000\000\260\074"
#define BITS_DBL_MIN     "\000\000\000\000\000\000\020\000"

static const double double_max = DBL_MAX;
static const double double_min = DBL_MIN;
static const double double_epsilon = DBL_EPSILON;

// double should be IEC 60559's double format, 53 bits precise.
// [C99:5.2.4.2.2p9]: the floating minima of FLT_RADIX to LDBL_MAX_10_EXP
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

int main(void)
{
    Test_FloatDouble();
    return _Test_Status();
}
