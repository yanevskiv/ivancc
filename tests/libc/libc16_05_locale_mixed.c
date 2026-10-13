// (Test) Status: 0
// A test file for <locale.h> categories of two locales.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// [C99:S7.11.1.1p8] LC_ALL's name should restore categories of two locales.
static void Test_LocaleMixed(void)
{
    char mixed[NAME_SIZE];
    char numeric[NAME_SIZE];

    if (! _TEST_TRUE(keep(start, LC_ALL))) {
        return;
    }
    _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL);
    _TEST_TRUE(keep(mixed, LC_ALL));
    _TEST_TRUE(keep(numeric, LC_NUMERIC));
    _TEST_STR(setlocale(LC_TIME, NULL), numeric);
    _TEST_TRUE(setlocale(LC_ALL, "C") != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
    _TEST_TRUE(setlocale(LC_ALL, mixed) != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), mixed);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
    _TEST_STR(setlocale(LC_ALL, NULL), start);
}

int main(void)
{
    Test_LocaleMixed();
    return _Test_Status();
}
