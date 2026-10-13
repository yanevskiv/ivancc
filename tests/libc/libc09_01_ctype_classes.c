// (Test) Status: 0
// A test file for <ctype.h> character classes.

// Include helpers.
#include "libc09_00_ctype_helpers.inc"

#define DIGIT "0123456789"
#define HEX   "abcdefABCDEF"
#define SPACE " \t\n\v\f\r"
#define BLANK " \t"
#define PUNCT "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
#define CNTRL "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\177"

// [C99:S7.4.1p1] A class should hold of its set alone: return the first to differ.
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

// [C99:S7.4.1p1] Each class should be true of its own characters and of no other.
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

int main(void)
{
    Test_CtypeClasses();
    return _Test_Status();
}
