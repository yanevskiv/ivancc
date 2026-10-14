// (Test) Status: 0
// A test file for <stdlib.h> getenv and system.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// getenv should give a variable's value, none for no other.
// [C99:7.20.4.5p2]: getenv
static void Test_StdlibEnvironment(void)
{
    const char *value = getenv("IVCC_TEST");

    _TEST_STR(value, "1");
    _TEST_TRUE((getenv)("IVCC_TEST") == value);
    _TEST_STR(getenv("IVCC"), NULL);
    _TEST_STR(getenv("IVCC_TEST_UNSET"), NULL);
    (void) system(NULL);
    (void) (system)(NULL);
}

int main(void)
{
    Test_StdlibEnvironment();
    return _Test_Status();
}
