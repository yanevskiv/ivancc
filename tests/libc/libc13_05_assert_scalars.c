// (Test) Status: 0
// A test file for <assert.h> scalar arguments.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

// assert should take a true scalar of any type, over lines too.
// [C99:7.2.1.1p2]: assert's message and abort
static void Test_AssertScalars(void)
{
    int i = -1;
    char ch = 'c';
    int *ptr = &i;
    double dbl = 0.5;
    const char *str = "s";
    unsigned long ul = ULONG_MAX;

    assert(i);
    assert(ch);
    assert(ul);
    assert(dbl);
    assert(str);
    assert(ptr);
    assert(*str == 's');
    assert((0, 1));
    assert(i < 0 ? 1 : 0);
    assert(i
           == -1);
    _TEST_INT((assert(1), FIVE), FIVE);
}

int main(void)
{
    Test_AssertScalars();
    return _Test_Status();
}
