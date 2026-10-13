// (Test) Status: 0
// A test file for <stdio.h> infinities and NaNs.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p8] An infinity or a NaN should begin with its text, after a sign.
static bool begins(const char *want, const char *format, double value)
{
    char got[HOLD];

    snprintf(got, sizeof(got), format, value);
    return _TEST_MEM(got, want, strlen(want));
}

// [C99/S7.19.6.1p6] An infinity or a NaN should fill its width with spaces alone.
static bool spaced(const char *format, double value, int width, size_t blank)
{
    int lost = 0;
    char got[HOLD];

    lost += ! _TEST_INT(snprintf(got, sizeof(got), format, value), width);
    lost += ! _TEST_INT(got[blank], ' ');
    lost += ! _TEST_TRUE(strchr(got, '0') == NULL);
    return lost == 0;
}

// [C99/S7.19.6.1p8] An infinity should be written inf, a NaN nan, or in capitals.
static void Test_StdioInfinite(void)
{
    double inf = strtod("inf", NULL);
    double nan = strtod("nan", NULL);

    _TEST_TRUE(begins("inf", "%f", inf));
    _TEST_TRUE(begins("inf", "%e", inf));
    _TEST_TRUE(begins("inf", "%g", inf));
    _TEST_TRUE(begins("-inf", "%f", -inf));
    _TEST_TRUE(begins("-inf", "%e", -inf));
    _TEST_TRUE(begins("-inf", "%g", -inf));
    _TEST_TRUE(begins("INF", "%F", inf));
    _TEST_TRUE(begins("INF", "%E", inf));
    _TEST_TRUE(begins("INF", "%G", inf));
    _TEST_TRUE(begins("-INF", "%F", -inf));

    _TEST_TRUE(begins("nan", "%f", nan));
    _TEST_TRUE(begins("nan", "%e", nan));
    _TEST_TRUE(begins("nan", "%g", nan));
    _TEST_TRUE(begins("NAN", "%F", nan));
    _TEST_TRUE(begins("NAN", "%E", nan));
    _TEST_TRUE(begins("NAN", "%G", nan));

    _TEST_TRUE(begins("+inf", "%+f", inf));
    _TEST_TRUE(begins(" inf", "% f", inf));
    _TEST_TRUE(begins("+inf", "%+ f", inf));
    _TEST_TRUE(begins("-inf", "%+f", -inf));
    _TEST_TRUE(begins("inf", "%.0f", inf));
    _TEST_TRUE(begins("inf", "%#.10f", inf));
    _TEST_TRUE(begins("inf", "%.3e", DBL_MAX * 2));
    _TEST_TRUE(begins("-inf", "%g", -DBL_MAX * 2));

    _TEST_TRUE(spaced("%20f", inf, 20, 0));
    _TEST_TRUE(spaced("%020f", inf, 20, 0));
    _TEST_TRUE(spaced("%020f", -inf, 20, 0));
    _TEST_TRUE(spaced("%+020e", inf, 20, 0));
    _TEST_TRUE(spaced("%020g", nan, 20, 0));
    _TEST_TRUE(spaced("%020F", nan, 20, 0));
    _TEST_TRUE(spaced("%-20f", inf, 20, 19));
    _TEST_TRUE(spaced("%-20f", nan, 20, 19));
    _TEST_TRUE(begins("inf", "%-20f", inf));
    _TEST_TRUE(begins("nan", "%-20g", nan));
}

int main(void)
{
    Test_StdioInfinite();
    return _Test_Status();
}
