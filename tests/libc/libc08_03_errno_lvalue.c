// (Test) Status: 0
// A test file for <errno.h> errno as an lvalue.

// Include helpers.
#include "libc08_00_errno_helpers.inc"

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

// [C99 S7.5p2] errno should be an int that keeps what the program stores in it.
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
    Test_ErrnoLvalue();
    return _Test_Status();
}
