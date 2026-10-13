// (Test) Status: 0
// A test file for <stdio.h> an encoding error of the lc and ls conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define BAD 0xe9

// [C99/S7.24.6.3.3p4] A wide character with no bytes should fail, errno EILSEQ.
static bool fails(const char *format, ...)
{
    int n;
    int lost = 0;
    va_list args;
    char got[SMALL];

    errno = 0;
    va_start(args, format);
    n = vsnprintf(got, sizeof(got), format, args);
    va_end(args);
    lost += ! _TEST_TRUE(n < 0);
    lost += ! _TEST_INT(errno, EILSEQ);
    return lost == 0;
}

// [C99/S7.19.6.1p14] printf should return a negative value at an encoding error.
static void Test_StdioWideError(void)
{
    char got[SMALL];
    char bytes[MB_LEN_MAX];
    const wchar_t wide[] = {L'a', BAD, L'\0'};

    if (! _TEST_INT(wctomb(bytes, BAD), -1)) {
        return;
    }
    _TEST_TRUE(fails("%lc", BAD));
    _TEST_TRUE(fails("%ls", wide));
    _TEST_TRUE(fails("%5lc", BAD));
    _TEST_TRUE(fails("%-5ls", wide));
    _TEST_TRUE(fails("ab%lcde", BAD));
    _TEST_TRUE(fails("%d%ls%d", 1, wide, 2));
    _TEST_TRUE(snprintf(NULL, 0, "%ls", wide) < 0);
    _TEST_TRUE(sprintf(got, "%ls", wide) < 0);

    _TEST_TRUE(prints("a", "%lc", L'a'));
    _TEST_TRUE(prints("a", "%ls", L"a"));
}

int main(void)
{
    Test_StdioWideError();
    return _Test_Status();
}
