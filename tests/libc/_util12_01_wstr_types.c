// (Test) Status: 0
// A test file for <_wstr.h> types against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <_wstr.h>

// A wchar_t should be the int _wstr works in, and a size_t what it counts in.
static void Test_WstrTypes(void)
{
    _Static_assert(sizeof(unsigned long) == sizeof(size_t), "_wstr counts in a size_t");
    _Static_assert(sizeof(int) == sizeof(wchar_t), "_wstr works in a wchar_t");
    _Static_assert((wchar_t) -1 < 0, "_wstr works in a signed type");
}

int main(void)
{
    Test_WstrTypes();
    return _Test_Status();
}
