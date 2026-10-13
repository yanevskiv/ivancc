// (Test) Status: 0
// A test file for <locale.h> restoring the startup locale.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99 S7.11.1.1p8] The name at startup should restore locale and formatting.
static void Test_LocaleRestored(void)
{
    if (! _TEST_TRUE(keep(start, LC_ALL))) {
        return;
    }
    _TEST_INT(refused(UTF8), NONE);
    _TEST_TRUE(setlocale(LC_ALL, start) != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
    _TEST_TRUE(plain());
}

int main(void)
{
    Test_LocaleRestored();
    return _Test_Status();
}
