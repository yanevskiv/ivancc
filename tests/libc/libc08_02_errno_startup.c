// (Test) Status: 0
// A test file for <errno.h> errno at program startup.

// Include helpers.
#include "libc08_00_errno_helpers.inc"

// errno should be zero before the program stores anything in it.
// [C99:7.5p3]: errno, zero at program startup
static void Test_ErrnoStartup(void)
{
    _TEST_INT(errno, 0);
}

int main(void)
{
    Test_ErrnoStartup();
    return _Test_Status();
}
