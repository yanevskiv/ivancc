// (Test) Status: 0
// A test file for <locale.h> calls by pointer and past a macro.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99:S7.1.4p1] A pointer and a call in parentheses should reach each function.
static void Test_LocaleCalls(void)
{
    struct lconv *(*conv)(void) = localeconv;
    char *(*set)(int, const char *) = setlocale;

    if (! _TEST_TRUE(keep(start, LC_ALL))) {
        return;
    }
    _TEST_TRUE(set(LC_TIME, "C") != NULL);
    _TEST_TRUE((setlocale)(LC_TIME, NULL) != NULL);
    _TEST_STR(set(LC_ALL, NULL), start);
    _TEST_STR((setlocale)(LC_ALL, LACKING), NULL);
    _TEST_STR(conv()->decimal_point, ".");
    _TEST_STR((localeconv)()->decimal_point, ".");
}

int main(void)
{
    Test_LocaleCalls();
    return _Test_Status();
}
