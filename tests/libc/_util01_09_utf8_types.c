// (Test) Status: 0
// A test file for <_utf8.h> types against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <_utf8.h>

// A wide character should be the signed int _utf8 holds one in.
static void Test_Utf8Types(void)
{
    _Static_assert(sizeof(int) == sizeof(wchar_t), "_utf8 holds a wchar_t in an int");
    _Static_assert((wchar_t) -1 < 0, "_utf8 holds a wchar_t in a signed type");
}

int main(void)
{
    Test_Utf8Types();
    return _Test_Status();
}
