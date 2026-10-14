// (Test) Status: 0
// A test file for <locale.h> the locale at program startup.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// The program should start in the "C" locale in each category.
// [C99:7.11.1.1p4]: the "C" locale at program startup
static void Test_LocaleStartup(void)
{
    _TEST_TRUE(keep(start, LC_ALL));
    _TEST_INT(refused(NULL), NONE);
    _TEST_TRUE(plain());
}

int main(void)
{
    Test_LocaleStartup();
    return _Test_Status();
}
