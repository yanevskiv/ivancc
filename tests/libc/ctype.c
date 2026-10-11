// (Test) Status: 0
// <ctype.h> declares isalnum, isalpha, isblank, iscntrl, isdigit, isgraph, islower, isprint, ispunct, isspace, isupper and isxdigit, each true of the characters of its class in the "C" locale and of no other (S7.4.1),
// and tolower and toupper, each mapping a letter of the one case to its letter in the other and any other character to itself (S7.4.2), over every value of unsigned char (S7.4p1),
// each a function a pointer reaches, and one a call reaches past any macro of its name, evaluating its argument once (S7.1.4p1).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

#define NONE (-1)

#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define DIGIT "0123456789"
#define HEX   "abcdefABCDEF"
#define SPACE " \t\n\v\f\r"
#define BLANK " \t"
#define PUNCT "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
#define CNTRL "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\177"

// Return a character's place in a set, or NONE where the set lacks it.
static int place(const char *set, int ch)
{
    int i;

    for (i = 0; set[i] != '\0'; i++) {
        if ((unsigned char) set[i] == ch) {
            return i;
        }
    }
    return NONE;
}

// Return the first character past null a class and its set differ on.
static int misjudged(int (*classify)(int), const char *set)
{
    int ch;

    for (ch = 1; ch <= UCHAR_MAX; ch++) {
        if ((classify(ch) != 0) != (place(set, ch) != NONE)) {
            return ch;
        }
    }
    return NONE;
}

// Return the first character a mapping does not take from a set to another.
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

// Each classification is true of its class's characters and of no other.
static void Test_CtypeClasses(void)
{
    _TEST_INT(misjudged(isalnum, UPPER LOWER DIGIT), NONE);
    _TEST_INT(misjudged(isalpha, UPPER LOWER), NONE);
    _TEST_INT(misjudged(isblank, BLANK), NONE);
    _TEST_INT(misjudged(iscntrl, CNTRL), NONE);
    _TEST_INT(misjudged(isdigit, DIGIT), NONE);
    _TEST_INT(misjudged(isgraph, UPPER LOWER DIGIT PUNCT), NONE);
    _TEST_INT(misjudged(islower, LOWER), NONE);
    _TEST_INT(misjudged(isprint, UPPER LOWER DIGIT PUNCT " "), NONE);
    _TEST_INT(misjudged(ispunct, PUNCT), NONE);
    _TEST_INT(misjudged(isspace, SPACE), NONE);
    _TEST_INT(misjudged(isupper, UPPER), NONE);
    _TEST_INT(misjudged(isxdigit, DIGIT HEX), NONE);
}

// The null character is a control character and of no other class.
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

// A mapping gives a letter's other case and leaves any other character.
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

// A plain call judges as the function does, and so does one past a macro.
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
    Test_CtypeClasses();
    Test_CtypeNull();
    Test_CtypeMapping();
    Test_CtypeCalls();
    return _Test_Status();
}
