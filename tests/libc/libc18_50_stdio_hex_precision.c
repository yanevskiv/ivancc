// (Test) Status: 0
// A test file for <stdio.h> %a's precision.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p8] %a should write a precision's hex digits after its point.
static bool counted(const char *format, double value, int digits, bool point)
{
    char *dot;
    char *mark;
    int lost = 0;
    char got[HOLD];

    snprintf(got, sizeof(got), format, value);
    dot = strchr(got, '.');
    mark = strchr(got, 'p');
    if (! _TEST_TRUE(mark != NULL)) {
        return false;
    }
    lost += ! _TEST_TRUE((dot != NULL) == point);
    if (dot != NULL) {
        lost += ! _TEST_INT((int) (mark - dot) - 1, digits);
        lost += ! _TEST_INT((int) strspn(dot + 1, "0123456789abcdef"), digits);
    }
    return lost == 0;
}

// [C99/S7.19.6.1p11] %a should round a value to the nearest of its precision.
static bool nearest(const char *format, double value, int digits)
{
    int k;
    double back;
    char got[HOLD];
    double bound = (value < 0 ? -value : value) / 2;

    for (k = 0; k < digits; k++) {
        bound /= 16;
    }
    snprintf(got, sizeof(got), format, value);
    back = strtod(got, NULL);
    return _TEST_TRUE((back < value ? value - back : back - value) <= bound);
}

// [C99/S7.19.6.1p8] %a should give back a value its precision holds all of.
static bool kept(const char *format, double value)
{
    char got[HOLD];

    snprintf(got, sizeof(got), format, value);
    return _TEST_TRUE(strtod(got, NULL) == value);
}

// [C99/S7.19.6.1p8] %a's precision should count its digits, a zero leave no point.
static void Test_StdioHexPrecision(void)
{
    char got[HOLD];

    _TEST_TRUE(counted("%.0a", 1.0, 0, false));
    _TEST_TRUE(counted("%.0a", 1.5, 0, false));
    _TEST_TRUE(counted("%.0a", 0.0, 0, false));
    _TEST_TRUE(counted("%.a", 1.5, 0, false));
    _TEST_TRUE(counted("%#.0a", 1.5, 0, true));
    _TEST_TRUE(counted("%#.0a", 0.0, 0, true));
    _TEST_TRUE(counted("%.1a", 1.5, 1, true));
    _TEST_TRUE(counted("%.1a", 1.0, 1, true));
    _TEST_TRUE(counted("%.3a", 0.1, 3, true));
    _TEST_TRUE(counted("%.5a", 0.0, 5, true));
    _TEST_TRUE(counted("%.13a", 0.1, 13, true));
    _TEST_TRUE(counted("%.20a", 0.1, 20, true));
    _TEST_TRUE(counted("%.40a", 1.0, 40, true));
    _TEST_TRUE(counted("%.3a", DBL_MAX, 3, true));
    _TEST_TRUE(counted("%.3a", DBL_MIN * DBL_EPSILON, 3, true));
    _TEST_TRUE(counted("%#.4a", -1.0, 4, true));

    _TEST_TRUE(nearest("%.0a", 1.5, 0));
    _TEST_TRUE(nearest("%.0a", 2.5, 0));
    _TEST_TRUE(nearest("%.0a", 1.4375, 0));
    _TEST_TRUE(nearest("%.0a", 1.9375, 0));
    _TEST_TRUE(nearest("%.1a", 1.03125, 1));
    _TEST_TRUE(nearest("%.1a", 1.09375, 1));
    _TEST_TRUE(nearest("%.1a", 1.96875, 1));
    _TEST_TRUE(nearest("%.3a", 0.1, 3));
    _TEST_TRUE(nearest("%.6a", 0.7, 6));
    _TEST_TRUE(nearest("%.12a", 0.1, 12));
    _TEST_TRUE(nearest("%.3a", -1e300, 3));
    _TEST_TRUE(nearest("%.3a", 1e-300, 3));

    _TEST_TRUE(kept("%.13a", 0.1));
    _TEST_TRUE(kept("%.20a", 0.7));
    _TEST_TRUE(kept("%.1a", 1.5));
    _TEST_TRUE(kept("%.0a", 4.0));
    _TEST_TRUE(kept("%.13a", DBL_MAX));
    _TEST_TRUE(kept("%.13a", DBL_MIN * DBL_EPSILON));

    _TEST_INT(snprintf(got, sizeof(got), "%.*a", 2, 1.5), snprintf(NULL, 0, "%.2a", 1.5));
    _TEST_TRUE(strtod(got, NULL) == 1.5);
    _TEST_INT(snprintf(got, sizeof(got), "%.*a", -1, 0.1), snprintf(NULL, 0, "%a", 0.1));
    _TEST_TRUE(strtod(got, NULL) == 0.1);
    _TEST_INT(snprintf(NULL, 0, "%.3000a", 1.5), snprintf(NULL, 0, "%.0a", 1.0) + 3001);
}

int main(void)
{
    Test_StdioHexPrecision();
    return _Test_Status();
}
