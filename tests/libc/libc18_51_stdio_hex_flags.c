// (Test) Status: 0
// A test file for <stdio.h> %a's flags and width.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p6] %a should begin as its flags say, fill a width and be exact.
static bool fills(const char *format, double value, int width, const char *first)
{
    char *end;
    int lost = 0;
    char got[HOLD];
    int n = snprintf(got, sizeof(got), format, value);

    lost += ! _TEST_INT(n, (int) strlen(got));
    lost += ! _TEST_TRUE(width == NONE || n == width);
    lost += ! _TEST_MEM(got, first, strlen(first));
    lost += ! _TEST_TRUE(strtod(got, &end) == value);
    lost += ! _TEST_INT((int) strspn(end, " "), (int) strlen(end));
    return lost == 0;
}

// [C99/S7.19.6.1p6] %a should take a sign, a space, zeros after its 0x and a side.
static void Test_StdioHexFlags(void)
{
    char got[HOLD];

    _TEST_TRUE(fills("%+a", 1.5, NONE, "+0x"));
    _TEST_TRUE(fills("%+a", -1.5, NONE, "-0x"));
    _TEST_TRUE(fills("% a", 1.5, NONE, " 0x"));
    _TEST_TRUE(fills("% a", -1.5, NONE, "-0x"));
    _TEST_TRUE(fills("%+ a", 1.5, NONE, "+0x"));
    _TEST_TRUE(fills("%+a", 0.0, NONE, "+0x"));
    _TEST_TRUE(fills("%+.3a", 1.5, NONE, "+0x"));

    _TEST_TRUE(fills("%20a", 1.5, 20, "  "));
    _TEST_TRUE(fills("%20a", -1.5, 20, "  "));
    _TEST_TRUE(fills("%+20a", 1.5, 20, "  "));
    _TEST_TRUE(fills("%2a", 1.5, NONE, "0x"));
    _TEST_TRUE(fills("%-20a", 1.5, 20, "0x"));
    _TEST_TRUE(fills("%-20a", -1.5, 20, "-0x"));
    _TEST_TRUE(fills("%-+20a", 1.5, 20, "+0x"));
    _TEST_TRUE(fills("%-20.3a", 1.5, 20, "0x"));

    _TEST_TRUE(fills("%020a", 1.5, 20, "0x0"));
    _TEST_TRUE(fills("%020a", -1.5, 20, "-0x0"));
    _TEST_TRUE(fills("%+020a", 1.5, 20, "+0x0"));
    _TEST_TRUE(fills("% 020a", 1.5, 20, " 0x0"));
    _TEST_TRUE(fills("%020.3a", 1.5, 20, "0x0"));
    _TEST_TRUE(fills("%-020a", 1.5, 20, "0x"));
    _TEST_TRUE(fills("%020a", 0.0, 20, "0x0"));

    snprintf(got, sizeof(got), "%-020a", 1.5);
    _TEST_TRUE(got[2] != '0');
    _TEST_INT(got[19], ' ');
    snprintf(got, sizeof(got), "%20a", 1.5);
    _TEST_INT(got[0], ' ');
    _TEST_TRUE(got[19] != ' ');

    _TEST_INT(snprintf(NULL, 0, "%*a", 20, 1.5), 20);
    _TEST_INT(snprintf(NULL, 0, "%*a|", -20, 1.5), 21);
    _TEST_INT(snprintf(NULL, 0, "%*.*a", 20, 3, 0.1), 20);
}

int main(void)
{
    Test_StdioHexFlags();
    return _Test_Status();
}
