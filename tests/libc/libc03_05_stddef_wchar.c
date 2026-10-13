// (Test) Status: 0
// A test file for <stddef.h> wchar_t type.

// Include helpers.
#include "libc03_00_stddef_helpers.inc"

// [C99/S7.17p2] wchar_t should be a wide character constant's type, and hold it.
static void Test_StddefWchar(void)
{
    wchar_t null = L'\0';
    wchar_t wide = L'a';

    _TEST_UINT(sizeof(wchar_t), sizeof(L'a'));
    _TEST_INT(wide, 'a');
    _TEST_INT(null, 0);
}

int main(void)
{
    Test_StddefWchar();
    return _Test_Status();
}
