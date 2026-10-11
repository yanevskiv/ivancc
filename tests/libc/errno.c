// (Test) Status: 0
// <errno.h> defines EDOM, EILSEQ and ERANGE, distinct positive integer constants of type int, each fit for #if (S7.5p2), and so for a case label,
// and errno, a modifiable lvalue of type int, zero at program startup (S7.5p3), which keeps what it is given, by a function and through a pointer too.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>

// Whether a directive finds the three macros positive and distinct.
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

// Tell the three macros apart, as only distinct constants let a switch do.
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

// Store a number in errno from another function.
static void set(int number)
{
    errno = number;
}

// Return errno as another function reads it.
static int get(void)
{
    return errno;
}

// errno is zero before the program has stored anything in it.
static void Test_ErrnoStartup(void)
{
    _TEST_INT(errno, 0);
}

// The three macros are distinct positive int constants, in a directive too.
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

// errno is an int that keeps what any part of the program stores in it.
static void Test_ErrnoLvalue(void)
{
    int *where = &errno;

    _TEST_UINT(sizeof(errno), sizeof(int));
    errno = EDOM;
    _TEST_INT(errno, EDOM);
    _TEST_INT(get(), EDOM);
    _TEST_INT(*where, EDOM);
    set(ERANGE);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(*where, ERANGE);
    *where = EILSEQ;
    _TEST_INT(errno, EILSEQ);
    _TEST_INT(get(), EILSEQ);
    errno++;
    _TEST_INT(errno, EILSEQ + 1);
    errno -= 1;
    _TEST_INT(errno, EILSEQ);
    _TEST_INT(errno = INT_MAX, INT_MAX);
    _TEST_INT(get(), INT_MAX);
    errno = INT_MIN;
    _TEST_INT(*where, INT_MIN);
    errno = 0;
    _TEST_INT(*where, 0);
    _TEST_TRUE(where == &errno);
}

int main(void)
{
    Test_ErrnoStartup();
    Test_ErrnoMacros();
    Test_ErrnoLvalue();
    return _Test_Status();
}
