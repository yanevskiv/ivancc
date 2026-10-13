// (Test) Status: 0
// A test file for <locale.h> a copy of localeconv's formatting.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99 S7.11.2.1p8] A copy of the formatting should be the program's to change.
static void Test_LocaleFormatting(void)
{
    struct lconv copy = *localeconv();

    _TEST_STR(copy.decimal_point, ".");
    _TEST_INT(copy.frac_digits, CHAR_MAX);
    copy.decimal_point = ",";
    copy.frac_digits = 2;
    _TEST_STR(copy.decimal_point, ",");
    _TEST_INT(copy.frac_digits, 2);
    _TEST_TRUE(plain());
}

int main(void)
{
    Test_LocaleFormatting();
    return _Test_Status();
}
