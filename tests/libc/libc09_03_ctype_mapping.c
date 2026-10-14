// (Test) Status: 0
// A test file for <ctype.h> case mappings.

// Include helpers.
#include "libc09_00_ctype_helpers.inc"

// A mapping should take one set to another: return the first to err.
// [C99:7.4.2]: tolower and toupper
static int mismapped(int (*map)(int), const char *from, const char *to)
{
    int at;
    int ch;

    for (ch = 0; ch <= UCHAR_MAX; ch++) {
        at = place(from, ch);
        if (map(ch) != (at == NONE ? ch : to[at])) {
            return ch;
        }
    }
    return NONE;
}

// A mapping should give a letter's other case and leave any other.
// [C99:7.4.2]: tolower and toupper
static void Test_CtypeMapping(void)
{
    _TEST_INT(mismapped(tolower, UPPER, LOWER), NONE);
    _TEST_INT(mismapped(toupper, LOWER, UPPER), NONE);
    _TEST_INT(tolower('Q'), 'q');
    _TEST_INT(tolower('q'), 'q');
    _TEST_INT(tolower('7'), '7');
    _TEST_INT(tolower('['), '[');
    _TEST_INT(toupper('q'), 'Q');
    _TEST_INT(toupper('Q'), 'Q');
    _TEST_INT(toupper('7'), '7');
    _TEST_INT(toupper('{'), '{');
}

int main(void)
{
    Test_CtypeMapping();
    return _Test_Status();
}
