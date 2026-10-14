// (Test) Status: 0
// A test file for <wchar.h> formatting characters and strings.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:7.24.2.1p8] %c should write its argument as btowc converts it.
static void Test_WcharWideConversionChar(void)
{
    wchar_t out[SIZE];

    _TEST_INT(swprintf(out, SIZE, L"%c%c%c", 'a', 'b', 'c'), 3);
    _TEST_MEM(out, L"abc", WIDE(4));
    _TEST_INT(swprintf(out, SIZE, L"%3c|%-3c|", 'a', 'b'), 8);
    _TEST_MEM(out, L"  a|b  |", WIDE(9));
    _TEST_INT(swprintf(out, SIZE, L"a%cb", '\0'), 3);
    _TEST_MEM(out, L"a\0b", WIDE(4));
}

// [C99:7.24.2.1p8] %lc should write its argument, a wide character, as it is.
static void Test_WcharWideConversionWideChar(void)
{
    wchar_t out[SIZE];
    static const wchar_t past[] = {0xE9, L' ', L' ', 0x20AC, L'|', 0x1F600, L' ', L'|', L'\0'};

    _TEST_INT(swprintf(out, SIZE, L"%lc%lc", (wint_t) L'a', (wint_t) L'b'), 2);
    _TEST_MEM(out, L"ab", WIDE(3));
    _TEST_INT(swprintf(out, SIZE, L"%lc%3lc|%-2lc|", (wint_t) 0xE9, (wint_t) 0x20AC, (wint_t) 0x1F600), 8);
    _TEST_MEM(out, past, sizeof(past));
}

// [C99:7.24.2.1p8] %s should write a string's characters, the precision's most.
static void Test_WcharWideConversionString(void)
{
    wchar_t out[SIZE];
    char bare[3] = {'x', 'y', 'z'};

    _TEST_INT(swprintf(out, SIZE, L"%s|%s|", "abc", ""), 5);
    _TEST_MEM(out, L"abc||", WIDE(6));
    _TEST_INT(swprintf(out, SIZE, L"%5s|%-5s|", "abc", "abc"), 12);
    _TEST_MEM(out, L"  abc|abc  |", WIDE(13));
    _TEST_INT(swprintf(out, SIZE, L"%.2s|%.0s|%.9s|", "abc", "abc", "abc"), 8);
    _TEST_MEM(out, L"ab||abc|", WIDE(9));
    _TEST_INT(swprintf(out, SIZE, L"%5.2s|%-5.*s|", "abc", 1, "abc"), 12);
    _TEST_MEM(out, L"   ab|a    |", WIDE(13));
    _TEST_INT(swprintf(out, SIZE, L"%.3s|", bare), 4);
    _TEST_MEM(out, L"xyz|", WIDE(5));
}

// [C99:7.24.2.1p8] %ls should write a wide string's characters as they are.
static void Test_WcharWideConversionWideString(void)
{
    wchar_t out[SIZE];
    wchar_t bare[2] = {0x20AC, L'x'};
    static const wchar_t wide[] = {L'h', 0xE9, 0x20AC, L'\0'};
    static const wchar_t past[] = {L'h', 0xE9, 0x20AC, L'|', L' ', L' ', L'h', 0xE9, 0x20AC, L'|', L'h', 0xE9, L' ', L' ', L'|', L'\0'};
    static const wchar_t cut[] = {0x20AC, L'x', L'|', L'\0'};

    _TEST_INT(swprintf(out, SIZE, L"%ls|%ls|", L"abc", L""), 5);
    _TEST_MEM(out, L"abc||", WIDE(6));
    _TEST_INT(swprintf(out, SIZE, L"%ls|%5ls|%-4.2ls|", wide, wide, wide), 15);
    _TEST_MEM(out, past, sizeof(past));
    _TEST_INT(swprintf(out, SIZE, L"%.2ls|", bare), 3);
    _TEST_MEM(out, cut, sizeof(cut));
}

int main(void)
{
    Test_WcharWideConversionChar();
    Test_WcharWideConversionWideChar();
    Test_WcharWideConversionString();
    Test_WcharWideConversionWideString();
    return _Test_Status();
}
