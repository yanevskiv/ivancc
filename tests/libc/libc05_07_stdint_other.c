// (Test) Status: 0
// A test file for <stdint.h> limits of other integer types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99:S7.18.3p2] The other types' limits should be those their sizes give.
static void Test_StdintOther(void)
{
    _TEST_INT(PTRDIFF_MIN, least(sizeof(ptrdiff_t)));
    _TEST_INT(PTRDIFF_MAX, most(sizeof(ptrdiff_t)));
    _TEST_UINT(SIZE_MAX, umost(sizeof(size_t)));
    _TEST_UINT(SIZE_MAX, (size_t) -1);
    _TEST_TRUE((wchar_t) -1 < 0);
    _TEST_INT(WCHAR_MIN, least(sizeof(wchar_t)));
    _TEST_INT(WCHAR_MAX, most(sizeof(wchar_t)));
    _TEST_INT(SIG_ATOMIC_MIN, INT_MIN);
    _TEST_INT(SIG_ATOMIC_MAX, INT_MAX);
    _TEST_INT(WINT_MIN, 0);
    _TEST_INT(WINT_MAX, 4294967295L);
}

int main(void)
{
    Test_StdintOther();
    return _Test_Status();
}
