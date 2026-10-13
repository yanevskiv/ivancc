// (Test) Status: 0
// A test file for <locale.h> selecting the "C" locale.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99:S7.11.1.1p6] Selecting "C" should give the name the program started with.
static void Test_LocaleSelected(void)
{
    if (! _TEST_TRUE(keep(start, LC_ALL))) {
        return;
    }
    _TEST_STR(setlocale(LC_ALL, "C"), start);
    _TEST_INT(refused("C"), NONE);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

int main(void)
{
    Test_LocaleSelected();
    return _Test_Status();
}
