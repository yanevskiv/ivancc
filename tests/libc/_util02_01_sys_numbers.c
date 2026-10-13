// (Test) Status: 0
// A test file for <_sys.h> numbers against C99's.

// Includes.
#include <_test.h>
#include <errno.h>
#include <stdio.h>
#include <_sys.h>

// C99's error numbers and seek origins should be Linux's, as _sys has them.
static void Test_SysNumbers(void)
{
    _Static_assert(EDOM == _SYS_EDOM, "EDOM is _SYS_EDOM");
    _Static_assert(EILSEQ == _SYS_EILSEQ, "EILSEQ is _SYS_EILSEQ");
    _Static_assert(ERANGE == _SYS_ERANGE, "ERANGE is _SYS_ERANGE");
    _Static_assert(SEEK_SET == _SYS_SEEK_SET, "SEEK_SET is _SYS_SEEK_SET");
    _Static_assert(SEEK_CUR == _SYS_SEEK_CUR, "SEEK_CUR is _SYS_SEEK_CUR");
    _Static_assert(SEEK_END == _SYS_SEEK_END, "SEEK_END is _SYS_SEEK_END");
}

int main(void)
{
    Test_SysNumbers();
    return _Test_Status();
}
