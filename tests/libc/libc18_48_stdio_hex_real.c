// (Test) Status: 0
// A test file for <stdio.h> %a.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p8] %a's text should be [-]0xh.hhhhp(+-)d, and give its value back.
static bool exact(double value)
{
    int n;
    char *end;
    int lost = 0;
    size_t k = 0;
    char got[HOLD];
    double size = value < 0 ? -value : value;

    n = snprintf(got, sizeof(got), "%a", value);
    lost += ! _TEST_INT(n, (int) strlen(got));
    lost += ! _TEST_TRUE(strtod(got, &end) == value);
    lost += ! _TEST_INT(end[0], '\0');
    k += got[k] == '-' ? 1 : 0;
    lost += ! _TEST_MEM(got + k, "0x", 2);
    k += 2;
    lost += ! _TEST_TRUE(isxdigit((unsigned char) got[k]));
    lost += ! _TEST_TRUE(size < DBL_MIN || got[k] != '0');
    lost += ! _TEST_TRUE(size != 0 || got[k] == '0');
    k++;
    if (got[k] == '.') {
        k++;
        k += strspn(got + k, "0123456789abcdef");
    }
    lost += ! _TEST_INT(got[k], 'p');
    k++;
    lost += ! _TEST_TRUE(got[k] == '+' || got[k] == '-');
    k++;
    lost += ! _TEST_TRUE(isdigit((unsigned char) got[k]));
    lost += ! _TEST_TRUE(got[k] != '0' || got[k + 1] == '\0');
    lost += ! _TEST_TRUE(size != 0 || got[k] == '0');
    lost += ! _TEST_INT((int) strspn(got + k, "0123456789"), (int) strlen(got + k));
    return lost == 0;
}

// [C99/S7.19.6.1p8] %a with no precision should write a double exactly, in hex.
static void Test_StdioHexReal(void)
{
    char got[HOLD];
    double tiny = DBL_MIN * DBL_EPSILON;

    _TEST_TRUE(exact(1.0));
    _TEST_TRUE(exact(1.5));
    _TEST_TRUE(exact(0.1));
    _TEST_TRUE(exact(-2.0));
    _TEST_TRUE(exact(-0.3));
    _TEST_TRUE(exact(255.0));
    _TEST_TRUE(exact(1e300));
    _TEST_TRUE(exact(1e-300));
    _TEST_TRUE(exact(3.141592653589793));
    _TEST_TRUE(exact(1.0 + DBL_EPSILON));
    _TEST_TRUE(exact(DBL_EPSILON));
    _TEST_TRUE(exact(DBL_MAX));
    _TEST_TRUE(exact(-DBL_MAX));
    _TEST_TRUE(exact(DBL_MIN));

    _TEST_TRUE(exact(DBL_MIN / 4));
    _TEST_TRUE(exact(DBL_MIN - tiny));
    _TEST_TRUE(exact(tiny));
    _TEST_TRUE(exact(-tiny));

    _TEST_TRUE(exact(0.0));
    _TEST_TRUE(prints("0x0p+0", "%.0a", 0.0));
    _TEST_TRUE(prints("-0x0p+0", "%.0a", -0.0));
    _TEST_TRUE(prints("0x0.000p+0", "%.3a", 0.0));

    _TEST_INT(snprintf(got, sizeof(got), "[%a][%d][%s]", 1.5, 7, "s"), snprintf(NULL, 0, "%a", 1.5) + 8);
    _TEST_STR(got + strlen(got) - 7, "][7][s]");
    _TEST_TRUE(strtod(got + 1, NULL) == 1.5);
}

int main(void)
{
    Test_StdioHexReal();
    return _Test_Status();
}
