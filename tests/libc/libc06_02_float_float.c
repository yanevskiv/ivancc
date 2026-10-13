// (Test) Status: 0
// A test file for <float.h> float characteristics.

// Include helpers.
#include "libc06_00_float_helpers.inc"

// [C99 S5.2.4.2.2p6] float should have IEC 60559's single format in #if.
#define DIRECTIVE_FLT 0
#if (FLT_MANT_DIG == 24) \
    && (FLT_DIG == 6) \
    && (FLT_MIN_EXP == -125) \
    && (FLT_MIN_10_EXP == -37) \
    && (FLT_MAX_EXP == 128) \
    && (FLT_MAX_10_EXP == 38)
#undef DIRECTIVE_FLT
#define DIRECTIVE_FLT 1
#endif

#define BITS_FLT_MAX     "\377\377\177\177"
#define BITS_FLT_EPSILON "\000\000\000\064"
#define BITS_FLT_MIN     "\000\000\200\000"

static const float float_max = FLT_MAX;
static const float float_min = FLT_MIN;
static const float float_epsilon = FLT_EPSILON;

// [C99 S5.2.4.2.2p9] float should be IEC 60559's single format, 24 bits precise.
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

int main(void)
{
    Test_FloatFloat();
    return _Test_Status();
}
