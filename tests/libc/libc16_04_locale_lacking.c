// (Test) Status: 0
// A test file for <locale.h> a locale setlocale lacks.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99/S7.11.1.1p6] A locale setlocale lacks should be refused, the locale kept.
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
