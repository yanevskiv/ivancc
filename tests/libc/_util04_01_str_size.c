// (Test) Status: 0
// A test file for <_str.h> counts against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <_str.h>

// A size_t should be the unsigned long _str counts in.
static void Test_StrSize(void)
{
    _Static_assert(sizeof(unsigned long) == sizeof(size_t), "_str counts in a size_t");
    _Static_assert((size_t) -1 > 0, "_str counts in an unsigned type");
}

int main(void)
{
    Test_StrSize();
    return _Test_Status();
}
