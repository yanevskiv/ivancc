// (Test) Status: 0
// A test file for <stddef.h> ptrdiff_t type.

// Include helpers.
#include "libc03_00_stddef_helpers.inc"

#define AHEAD 2

// [C99/S7.17p2] ptrdiff_t should be signed and hold two pointers' difference.
static void Test_StddefPtrdiff(void)
{
    int numbers[TAIL];
    int *start = &numbers[0];
    int *ahead = &numbers[AHEAD];
    ptrdiff_t back = start - ahead;
    ptrdiff_t forth = ahead - start;

    _TEST_TRUE((ptrdiff_t) -1 < 0);
    _TEST_UINT(sizeof(ptrdiff_t), sizeof(ahead - start));
    _TEST_INT(forth, AHEAD);
    _TEST_INT(back, -AHEAD);
}

int main(void)
{
    Test_StddefPtrdiff();
    return _Test_Status();
}
