// (Test) Status: 0
// A test file for <stdio.h> %A.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p8] %A should write what %a does, each of its letters a capital.
static bool capital(const char *lower, const char *upper, double value)
{
    size_t k;
    int lost = 0;
    char got[HOLD];
    char want[HOLD];

    lost += ! _TEST_INT(snprintf(got, sizeof(got), upper, value), snprintf(want, sizeof(want), lower, value));
    for (k = 0; want[k] != '\0'; k++) {
        want[k] = (char) toupper((unsigned char) want[k]);
    }
    lost += ! _TEST_STR(got, want);
    return lost == 0;
}

// [C99/S7.19.6.1p8] %A should write X, P and ABCDEF for %a's x, p and abcdef.
static void Test_StdioHexCapitals(void)
{
    char got[HOLD];
    double inf = strtod("inf", NULL);

    _TEST_TRUE(capital("%a", "%A", 1.0));
    _TEST_TRUE(capital("%a", "%A", 0.1));
    _TEST_TRUE(capital("%a", "%A", -255.5));
    _TEST_TRUE(capital("%a", "%A", 1e-300));
    _TEST_TRUE(capital("%a", "%A", DBL_MAX));
    _TEST_TRUE(capital("%a", "%A", 0.0));
    _TEST_TRUE(capital("%.3a", "%.3A", 0.1));
    _TEST_TRUE(capital("%.20a", "%.20A", 0.7));
    _TEST_TRUE(capital("%#.0a", "%#.0A", 1.0));
    _TEST_TRUE(capital("%+20a", "%+20A", 0.3));
    _TEST_TRUE(capital("%-20a|", "%-20A|", 0.3));
    _TEST_TRUE(capital("%020a", "%020A", -0.3));
    _TEST_TRUE(capital("%la", "%lA", 0.3));
    _TEST_TRUE(capital("%a", "%A", inf));

    snprintf(got, sizeof(got), "%A", 0.7);
    _TEST_TRUE(strstr(got, "0X") != NULL);
    _TEST_TRUE(strchr(got, 'P') != NULL);
    _TEST_TRUE(strpbrk(got, "xpabcdef") == NULL);
    _TEST_TRUE(strtod(got, NULL) == 0.7);
}

int main(void)
{
    Test_StdioHexCapitals();
    return _Test_Status();
}
