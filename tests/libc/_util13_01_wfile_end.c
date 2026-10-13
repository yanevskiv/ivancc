// (Test) Status: 0
// A test file for <_wfile.h> end and pushback against C99's.

// Includes.
#include <_test.h>
#include <stdio.h>
#include <wchar.h>
#include <_file.h>
#include <_utf8.h>
#include <_wfile.h>

// A stream's end should be WEOF and EOF, and it take back a whole character.
static void Test_WfileEnd(void)
{
    _Static_assert(_WFILE_END == WEOF, "_WFILE_END is WEOF");
    _Static_assert(_FILE_END == EOF, "_FILE_END is EOF");
    _Static_assert(_FILE_BACKS >= _UTF8_MAX, "a stream takes back the longest character's bytes");
}

int main(void)
{
    Test_WfileEnd();
    return _Test_Status();
}
