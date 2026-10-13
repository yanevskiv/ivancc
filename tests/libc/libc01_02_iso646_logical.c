// (Test) Status: 0
// A test file for <iso646.h> logical operators.

// Include helpers.
#include "libc01_00_iso646_helpers.inc"

static int calls;

// Count a call and return 1, to tell whether an operand was evaluated.
static int called(void)
{
    calls++;
    return 1;
}

// [C99:S7.9p1] and, or and not should yield 0 or 1, and skip an operand unneeded.
static void Test_Iso646Logical(void)
{
    _TEST_INT(1 and 2, 1);
    _TEST_INT(1 and 0, 0);
    _TEST_INT(0 or 0, 0);
    _TEST_INT(0 or THREE, 1);
    _TEST_INT(not 0, 1);
    _TEST_INT(not THREE, 0);
    _TEST_INT(1 or 0 and 0, 1);
    _TEST_INT(not 1 or 1, 1);
    _TEST_INT(1 not_eq 1, 0);
    _TEST_INT(1 not_eq 2, 1);

    _TEST_INT(0 and called(), 0);
    _TEST_INT(1 or called(), 1);
    _TEST_INT(calls, 0);
    _TEST_INT(1 and called(), 1);
    _TEST_INT(0 or called(), 1);
    _TEST_INT(calls, 2);
}

int main(void)
{
    Test_Iso646Logical();
    return _Test_Status();
}
