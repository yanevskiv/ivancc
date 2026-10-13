// (Test) Status: 0
// A test file for <errno.h> macros.

// Include helpers.
#include "libc08_00_errno_helpers.inc"

// [C99 S7.5p2] EDOM, EILSEQ and ERANGE should be positive and distinct in #if.
#define DIRECTIVE 0
#if (EDOM > 0) \
    && (EILSEQ > 0) \
    && (ERANGE > 0) \
    && (EDOM != EILSEQ) \
    && (EDOM != ERANGE) \
    && (EILSEQ != ERANGE)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

#define OTHER  0
#define FIRST  1
#define SECOND 2
#define THIRD  3

// [C99 S7.5p2] The three macros should be distinct, as a switch's labels must be.
static int place(int number)
{
    switch (number) {
    case EDOM:
        return FIRST;
    case EILSEQ:
        return SECOND;
    case ERANGE:
        return THIRD;
    default:
        return OTHER;
    }
}

// [C99 S7.5p2] The three macros should be distinct positive int constants.
static void Test_ErrnoMacros(void)
{
    _TEST_TRUE(EDOM > 0);
    _TEST_TRUE(EILSEQ > 0);
    _TEST_TRUE(ERANGE > 0);
    _TEST_TRUE(EDOM != EILSEQ);
    _TEST_TRUE(EDOM != ERANGE);
    _TEST_TRUE(EILSEQ != ERANGE);
    _TEST_UINT(sizeof(EDOM), sizeof(int));
    _TEST_UINT(sizeof(EILSEQ), sizeof(int));
    _TEST_UINT(sizeof(ERANGE), sizeof(int));
    _TEST_INT(DIRECTIVE, 1);
    _TEST_INT(place(EDOM), FIRST);
    _TEST_INT(place(EILSEQ), SECOND);
    _TEST_INT(place(ERANGE), THIRD);
    _TEST_INT(place(0), OTHER);
}

int main(void)
{
    Test_ErrnoMacros();
    return _Test_Status();
}
