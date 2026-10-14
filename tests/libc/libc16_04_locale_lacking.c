// (Test) Status: 0
// A test file for <locale.h> a locale setlocale lacks.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// A locale setlocale lacks should be refused, the locale kept.
// [C99:7.11.1.1p6]: setlocale's result, a name for the locale or a null pointer
static void Test_LocaleLacking(void)
{
    if (! _TEST_TRUE(keep(start, LC_ALL))) {
        return;
    }
    _TEST_STR(setlocale(LC_ALL, LACKING), NULL);
    _TEST_STR(setlocale(LC_CTYPE, LACKING), NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

int main(void)
{
    Test_LocaleLacking();
    return _Test_Status();
}
