// (Test) Status: 0
// A test file for <_file.h> macros against <stdio.h>'s.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdio.h>
#include <_file.h>

// stdio.h's macros should be _file's, and a temporary name fit its size.
static void Test_FileMacros(void)
{
    _Static_assert(_IOFBF == _FILE_FULL, "_IOFBF is _FILE_FULL");
    _Static_assert(_IOLBF == _FILE_LINE, "_IOLBF is _FILE_LINE");
    _Static_assert(_IONBF == _FILE_NONE, "_IONBF is _FILE_NONE");
    _Static_assert(EOF == _FILE_END, "EOF is _FILE_END");
    _Static_assert(BUFSIZ == _FILE_BUFFER, "BUFSIZ is _FILE_BUFFER");
    _Static_assert(L_tmpnam == _FILE_TEMP_SIZE, "L_tmpnam is _FILE_TEMP_SIZE");
    _Static_assert(sizeof(unsigned long) == sizeof(size_t), "_file counts in a size_t");
    _Static_assert(sizeof(_FILE_TEMP_LETTERS) - 1 == _FILE_TEMP_RADIX, "a temporary name's letters are its radix");
    _Static_assert(sizeof(_FILE_TEMP_PREFIX) + _FILE_TEMP_DIGITS <= _FILE_TEMP_SIZE, "a temporary name fits its size");
}

int main(void)
{
    Test_FileMacros();
    return _Test_Status();
}
