// (Test) Status: 0
// A test file for <stdio.h> the longest floating conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.12p3] A long text should have its length, its start and its end.
static bool spans(int len, const char *first, const char *last, const char *format, ...)
{
    int n;
    int lost = 0;
    va_list args;
    char *got = (char *) wide;

    va_start(args, format);
    n = vsnprintf(got, sizeof(wide), format, args);
    va_end(args);
    lost += ! _TEST_INT(n, len);
    lost += ! _TEST_INT((int) strlen(got), len);
    lost += ! _TEST_MEM(got, first, strlen(first));
    lost += ! _TEST_STR(got + strlen(got) - strlen(last), last);
    return lost == 0;
}

// [C99:7.19.6.1p13] A value's digits past DECIMAL_DIG should be exact, as glibc's.
static void Test_StdioExtremes(void)
{
    double tiny = DBL_MIN * DBL_EPSILON;
    long double small = LDBL_MIN * LDBL_EPSILON;

    _TEST_TRUE(spans(316, "179769313486231570814527423731", "04026184124858368.000000", "%f", DBL_MAX));
    _TEST_TRUE(spans(309, "179769313486231570814527423731", "881250404026184124858368", "%.0f", DBL_MAX));
    _TEST_TRUE(spans(310, "-17976931348623157081452742373", "881250404026184124858368", "%.0f", -DBL_MAX));
    _TEST_TRUE(spans(1024, "0.0000000000000000000000000000", "617210924625396728515625", "%.1022f", DBL_MIN));
    _TEST_TRUE(spans(1076, "0.0000000000000000000000000000", "506419718265533447265625", "%.1074f", tiny));
    _TEST_TRUE(spans(1082, "0.0000000000000000000000000000", "718265533447265625000000", "%.1080f", tiny));
    _TEST_TRUE(spans(757, "4.9406564584124654417656879286", "9718265533447265625e-324", "%.750e", tiny));
    _TEST_TRUE(spans(757, "4.9406564584124654417656879286", "9718265533447265625e-324", "%.800g", tiny));
    _TEST_TRUE(spans(13, "4.940656e-324", "4.940656e-324", "%e", tiny));

    _TEST_TRUE(spans(4940, "118973149535723176502126385303", "52086811989770240.000000", "%Lf", LDBL_MAX));
    _TEST_TRUE(spans(8008, "3.6451995318824746025284059336", "213552275207666157e-4951", "%.8000Le", small));

    _TEST_INT(snprintf(NULL, 0, "%f", DBL_MAX), 316);
    _TEST_INT(snprintf(NULL, 0, "%.3000f", 1.5), 3002);
    _TEST_TRUE(spans(3002, "1.5000000000", "0000000000", "%.3000f", 1.5));
    _TEST_TRUE(spans(3000, "          ", "  1.500000", "%3000f", 1.5));
    _TEST_TRUE(spans(3000, "-000000000", "001.500000", "%03000f", -1.5));
}

int main(void)
{
    Test_StdioExtremes();
    return _Test_Status();
}
