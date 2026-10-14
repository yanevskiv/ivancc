// (Test) Status: 0
// A test file for <stdint.h> intptr_t and uintptr_t types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// A pointer to void should survive intptr_t and uintptr_t.
// [C99:7.18.1.4p1]: intptr_t and uintptr_t, holding a pointer to void
static void Test_StdintPointer(void)
{
    int number = 0;
    void *ptr = &number;
    intptr_t held = (intptr_t) ptr;
    uintptr_t uheld = (uintptr_t) ptr;

    _TEST_TRUE((void *) held == ptr);
    _TEST_TRUE((void *) uheld == ptr);
    _TEST_TRUE((intptr_t) -1 < 0);
    _TEST_TRUE((uintptr_t) -1 > 0);
    _TEST_INT(INTPTR_MIN, least(sizeof(intptr_t)));
    _TEST_INT(INTPTR_MAX, most(sizeof(intptr_t)));
    _TEST_UINT(UINTPTR_MAX, umost(sizeof(uintptr_t)));
}

int main(void)
{
    Test_StdintPointer();
    return _Test_Status();
}
