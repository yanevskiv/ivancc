// (Test) Status: 0
// A test file for <stdbool.h> bool increments and decrements.

// Include helpers.
#include "libc02_00_stdbool_helpers.inc"

// true incremented should stay true, false decremented be true.
// [C99:6.3.1.2p1]: conversion to _Bool, 0 or 1
static void Test_StdboolSteps(void)
{
    bool flag = false;

    flag++;
    _TEST_INT(flag, 1);
    flag++;
    _TEST_INT(flag, 1);
    flag--;
    _TEST_INT(flag, 0);
    flag--;
    _TEST_INT(flag, 1);
}

int main(void)
{
    Test_StdboolSteps();
    return _Test_Status();
}
