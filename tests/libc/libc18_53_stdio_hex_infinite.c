// (Test) Status: 0
// A test file for <stdio.h> %a of infinities and NaNs.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// Two formats should write one value alike, and return alike.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static bool alike(const char *hex, const char *fixed, ...)
{
    int n1;
    int n2;
    int lost = 0;
    va_list args;
    va_list again;
    char got[HOLD];
    char want[HOLD];

    va_start(args, fixed);
    va_copy(again, args);
    n1 = vsnprintf(got, sizeof(got), hex, args);
    n2 = vsnprintf(want, sizeof(want), fixed, again);
    va_end(again);
    va_end(args);
    lost += ! _TEST_STR(got, want);
    lost += ! _TEST_INT(n1, n2);
    return lost == 0;
}

// %a should write an infinity or a NaN as %f does, and %A as %F.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static void Test_StdioHexInfinite(void)
{
    double inf = strtod("inf", NULL);
    double nan = strtod("nan", NULL);
    long double linf = strtold("inf", NULL);
    long double lnan = strtold("nan", NULL);

    _TEST_TRUE(alike("%a", "%f", inf));
    _TEST_TRUE(alike("%a", "%f", -inf));
    _TEST_TRUE(alike("%A", "%F", inf));
    _TEST_TRUE(alike("%A", "%F", -inf));
    _TEST_TRUE(alike("%a", "%f", nan));
    _TEST_TRUE(alike("%A", "%F", nan));
    _TEST_TRUE(alike("%a", "%f", DBL_MAX * 2));

    _TEST_TRUE(alike("%+a", "%+f", inf));
    _TEST_TRUE(alike("% a", "% f", inf));
    _TEST_TRUE(alike("%+a", "%+f", -inf));
    _TEST_TRUE(alike("%.0a", "%.0f", inf));
    _TEST_TRUE(alike("%.10a", "%.10f", inf));
    _TEST_TRUE(alike("%#a", "%#f", inf));
    _TEST_TRUE(alike("%#.0a", "%#.0f", nan));

    _TEST_TRUE(alike("%20a", "%20f", inf));
    _TEST_TRUE(alike("%020a", "%020f", inf));
    _TEST_TRUE(alike("%020a", "%020f", -inf));
    _TEST_TRUE(alike("%+020A", "%+020F", inf));
    _TEST_TRUE(alike("%020a", "%020f", nan));
    _TEST_TRUE(alike("%-20a|", "%-20f|", inf));
    _TEST_TRUE(alike("%-20a|", "%-20f|", nan));
    _TEST_TRUE(alike("%*a", "%*f", 12, inf));

    _TEST_TRUE(alike("%La", "%Lf", linf));
    _TEST_TRUE(alike("%La", "%Lf", -linf));
    _TEST_TRUE(alike("%LA", "%LF", linf));
    _TEST_TRUE(alike("%La", "%Lf", lnan));
    _TEST_TRUE(alike("%LA", "%LF", lnan));
    _TEST_TRUE(alike("%20La", "%20Lf", linf));
    _TEST_TRUE(alike("%020La", "%020Lf", lnan));
    _TEST_TRUE(alike("%La", "%Lf", LDBL_MAX * 2));
}

int main(void)
{
    Test_StdioHexInfinite();
    return _Test_Status();
}
