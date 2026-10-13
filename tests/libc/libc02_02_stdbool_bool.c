// (Test) Status: 0
// A test file for <stdbool.h> bool type.

// Include helpers.
#include "libc02_00_stdbool_helpers.inc"

// [C99:S7.16p2] bool should be _Bool, an unsigned type that holds true and false.
static void Test_StdboolBool(void)
{
    bool no = false;
    bool yes = true;

    _TEST_UINT(sizeof(bool), sizeof(_Bool));
    _TEST_INT(no, 0);
    _TEST_INT(yes, 1);
    _TEST_TRUE(yes);
    _TEST_TRUE(! no);
    _TEST_TRUE((bool) -1 > 0);
}

int main(void)
{
    Test_StdboolBool();
    return _Test_Status();
}
