// (Test) Status: 0
// A test file for <stddef.h> NULL macro.

// Include helpers.
#include "libc03_00_stddef_helpers.inc"

// [C99/S7.17p3] NULL should be a null pointer constant, equal to 0 and no object.
static void Test_StddefNull(void)
{
    int number = 0;
    int *some = &number;
    char *none = NULL;
    void (*func)(void) = NULL;

    _TEST_TRUE(NULL == 0);
    _TEST_TRUE((void *) NULL == (void *) 0);
    _TEST_TRUE(none == NULL);
    _TEST_TRUE(func == NULL);
    _TEST_TRUE(! none);
    _TEST_TRUE(! func);
    _TEST_TRUE(some != NULL);
    some = NULL;
    _TEST_TRUE(some == none);
}

int main(void)
{
    Test_StddefNull();
    return _Test_Status();
}
