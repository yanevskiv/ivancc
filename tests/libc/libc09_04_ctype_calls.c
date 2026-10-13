// (Test) Status: 0
// A test file for <ctype.h> calls past a macro.

// Include helpers.
#include "libc09_00_ctype_helpers.inc"

// [C99:S7.1.4p1] A plain call and one past a macro should judge as the function.
static void Test_CtypeCalls(void)
{
    int ch = '0';

    _TEST_TRUE(isalnum('a') && isalnum('7') && ! isalnum('_'));
    _TEST_TRUE(isalpha('a') && isalpha('Z') && ! isalpha('7'));
    _TEST_TRUE(isblank(' ') && isblank('\t') && ! isblank('\n'));
    _TEST_TRUE(iscntrl('\n') && iscntrl('\177') && ! iscntrl(' '));
    _TEST_TRUE(isdigit('0') && isdigit('9') && ! isdigit('a'));
    _TEST_TRUE(isgraph('a') && isgraph('~') && ! isgraph(' '));
    _TEST_TRUE(islower('a') && islower('z') && ! islower('A'));
    _TEST_TRUE(isprint('a') && isprint(' ') && ! isprint('\t'));
    _TEST_TRUE(ispunct('!') && ispunct('~') && ! ispunct('a'));
    _TEST_TRUE(isspace(' ') && isspace('\v') && ! isspace('a'));
    _TEST_TRUE(isupper('A') && isupper('Z') && ! isupper('a'));
    _TEST_TRUE(isxdigit('9') && isxdigit('f') && isxdigit('F') && ! isxdigit('g'));

    _TEST_TRUE((isalnum)('a') && ! (isalnum)('_'));
    _TEST_TRUE((isalpha)('a') && ! (isalpha)('7'));
    _TEST_TRUE((isblank)(' ') && ! (isblank)('\n'));
    _TEST_TRUE((iscntrl)('\n') && ! (iscntrl)(' '));
    _TEST_TRUE((isdigit)('7') && ! (isdigit)('a'));
    _TEST_TRUE((isgraph)('a') && ! (isgraph)(' '));
    _TEST_TRUE((islower)('a') && ! (islower)('A'));
    _TEST_TRUE((isprint)(' ') && ! (isprint)('\t'));
    _TEST_TRUE((ispunct)('!') && ! (ispunct)('a'));
    _TEST_TRUE((isspace)('\v') && ! (isspace)('a'));
    _TEST_TRUE((isupper)('A') && ! (isupper)('a'));
    _TEST_TRUE((isxdigit)('f') && ! (isxdigit)('g'));
    _TEST_INT((tolower)('Q'), 'q');
    _TEST_INT((toupper)('q'), 'Q');

    _TEST_TRUE(isdigit(ch++));
    _TEST_INT(ch, '1');
    _TEST_INT(toupper(ch++), '1');
    _TEST_INT(ch, '2');
}

int main(void)
{
    Test_CtypeCalls();
    return _Test_Status();
}
