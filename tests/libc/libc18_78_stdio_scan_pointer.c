// (Test) Status: 0
// A test file for <stdio.h> the p conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.2p12] %p should read back a pointer that %p wrote, equal to it.
static void Test_StdioScanPointer(void)
{
    int object;
    char ch = FILL;
    char text[HOLD];
    void *got = NULL;
    void *want = &object;

    _TEST_TRUE(sprintf(text, "%p", want) > 0);
    _TEST_INT(sscanf(text, "%p", &got), 1);
    _TEST_TRUE(got == want);
    got = NULL;
    _TEST_TRUE(sprintf(text, " \n%p!", want) > 0);
    _TEST_INT(sscanf(text, "%p%c", &got, &ch), 2);
    _TEST_TRUE(got == want);
    _TEST_INT(ch, '!');
    _TEST_TRUE(sprintf(text, "%p", (void *) NULL) > 0);
    _TEST_INT(sscanf(text, "%p", &got), 1);
    _TEST_TRUE(got == NULL);
    _TEST_TRUE(sprintf(text, "%p", (void *) text) > 0);
    _TEST_INT(sscanf(text, "%*p%p", &got), EOF);
    _TEST_TRUE(got == NULL);
}

int main(void)
{
    Test_StdioScanPointer();
    return _Test_Status();
}
