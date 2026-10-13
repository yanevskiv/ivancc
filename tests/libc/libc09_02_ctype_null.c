// (Test) Status: 0
// A test file for <ctype.h> classes of the null character.

// Include helpers.
#include "libc09_00_ctype_helpers.inc"

// [C99:S7.4.1p1] The null character should be of the control class alone.
static void Test_CtypeNull(void)
{
    _TEST_TRUE(! isalnum('\0'));
    _TEST_TRUE(! isalpha('\0'));
    _TEST_TRUE(! isblank('\0'));
    _TEST_TRUE(iscntrl('\0'));
    _TEST_TRUE(! isdigit('\0'));
    _TEST_TRUE(! isgraph('\0'));
    _TEST_TRUE(! islower('\0'));
    _TEST_TRUE(! isprint('\0'));
    _TEST_TRUE(! ispunct('\0'));
    _TEST_TRUE(! isspace('\0'));
    _TEST_TRUE(! isupper('\0'));
    _TEST_TRUE(! isxdigit('\0'));
}

int main(void)
{
    Test_CtypeNull();
    return _Test_Status();
}
