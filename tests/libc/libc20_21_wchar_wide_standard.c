// (Test) Status: 0
// (Test) Input:
// | ab
// | cd
// (Test) Output:
// | xy
// | wide
// A test file for <wchar.h> wide characters of the standard streams.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:7.24.3.7] getwchar and putwchar should be getwc and putwc of the two.
static void Test_WcharWideStandard(void)
{
    wchar_t line[SIZE];

    _TEST_INT(fwide(stdin, 0), 0);
    _TEST_INT(fwide(stdout, 0), 0);
    _TEST_TRUE(getwchar() == L'a');
    _TEST_TRUE((getwchar)() == L'b');
    _TEST_TRUE(fwide(stdin, 0) > 0);
    _TEST_TRUE(ungetwc(L'q', stdin) == L'q');
    _TEST_TRUE(getwc(stdin) == L'q');
    _TEST_TRUE(getwchar() == L'\n');
    _TEST_TRUE(fgetws(line, SIZE, stdin) == line);
    _TEST_MEM(line, L"cd\n", WIDE(4));
    _TEST_TRUE(getwchar() == WEOF);
    _TEST_TRUE(feof(stdin));

    _TEST_TRUE(putwchar(L'x') == L'x');
    _TEST_TRUE((putwchar)(L'y') == L'y');
    _TEST_TRUE(fwide(stdout, 0) > 0);
    _TEST_TRUE(putwc(L'\n', stdout) == L'\n');
    _TEST_TRUE(fputws(L"wide\n", stdout) >= 0);
    _TEST_INT(fflush(stdout), 0);
    _TEST_TRUE(! ferror(stdout));
}

int main(void)
{
    Test_WcharWideStandard();
    return _Test_Status();
}
