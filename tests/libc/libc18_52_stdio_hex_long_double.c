// (Test) Status: 0
// A test file for <stdio.h> the L and l lengths on %a.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p8] %La's text should begin [-]0x and give its long double back.
static bool back(long double value)
{
    char *end;
    int lost = 0;
    char got[HOLD];
    int n = snprintf(got, sizeof(got), "%La", value);

    lost += ! _TEST_INT(n, (int) strlen(got));
    lost += ! _TEST_MEM(got + (got[0] == '-' ? 1 : 0), "0x", 2);
    lost += ! _TEST_TRUE(strchr(got, 'p') != NULL);
    lost += ! _TEST_TRUE(strtold(got, &end) == value);
    lost += ! _TEST_INT(end[0], '\0');
    return lost == 0;
}

// [C99/S7.19.6.1p7] L should give %a a long double argument, and l change nothing.
static void Test_StdioHexLongDouble(void)
{
    char one[HOLD];
    char two[HOLD];
    long double small = LDBL_MIN * LDBL_EPSILON;

    _TEST_TRUE(back(1.0L));
    _TEST_TRUE(back(1.5L));
    _TEST_TRUE(back(0.1L));
    _TEST_TRUE(back(-2.0L));
    _TEST_TRUE(back(15.0L));
    _TEST_TRUE(back(16.0L));
    _TEST_TRUE(back(3.14159265358979323846L));
    _TEST_TRUE(back(1.0L + LDBL_EPSILON));
    _TEST_TRUE(back(LDBL_EPSILON));
    _TEST_TRUE(back(1e600L));
    _TEST_TRUE(back(-1e-600L));
    _TEST_TRUE(back(LDBL_MAX));
    _TEST_TRUE(back(LDBL_MIN));
    _TEST_TRUE(back(LDBL_MIN / 4));
    _TEST_TRUE(back(small));
    _TEST_TRUE(back(0.0L));

    _TEST_TRUE(prints("0x0p+0", "%.0La", 0.0L));
    _TEST_TRUE(prints("-0x0.00p+0", "%.2La", -0.0L));

    snprintf(one, sizeof(one), "%.3La", 0.1L);
    _TEST_INT((int) (strchr(one, 'p') - strchr(one, '.')), 4);
    snprintf(one, sizeof(one), "%.20La", 0.1L);
    _TEST_TRUE(strtold(one, NULL) == 0.1L);
    snprintf(one, sizeof(one), "%+30La", 1.5L);
    _TEST_INT((int) strlen(one), 30);
    _TEST_TRUE(strtold(one, NULL) == 1.5L);
    snprintf(one, sizeof(one), "%LA", 0.7L);
    _TEST_TRUE(strpbrk(one, "xpabcdef") == NULL);
    _TEST_TRUE(strtold(one, NULL) == 0.7L);

    snprintf(one, sizeof(one), "%la", 0.7);
    snprintf(two, sizeof(two), "%a", 0.7);
    _TEST_STR(one, two);
    snprintf(one, sizeof(one), "%lA", 0.7);
    snprintf(two, sizeof(two), "%A", 0.7);
    _TEST_STR(one, two);
    snprintf(one, sizeof(one), "[%La][%d][%a]", 1.5L, 7, 2.5);
    _TEST_TRUE(strstr(one, "][7][") != NULL);
    _TEST_TRUE(strtold(one + 1, NULL) == 1.5L);
    _TEST_TRUE(strtod(strstr(one, "][7][") + 5, NULL) == 2.5);
}

int main(void)
{
    Test_StdioHexLongDouble();
    return _Test_Status();
}
