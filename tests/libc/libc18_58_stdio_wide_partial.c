// (Test) Status: 0
// A test file for <stdio.h> a precision of the ls conversion in bytes.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p8] %ls should write the whole characters a precision's bytes hold.
static void Test_StdioWidePartial(void)
{
    const wchar_t open[2] = {0xe9, 0x20ac};
    const wchar_t mixed[] = {L'a', 0xe9, 0x20ac, L'\0'};

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(prints("", "%.0ls", mixed));
    _TEST_TRUE(prints("a", "%.1ls", mixed));
    _TEST_TRUE(prints("a", "%.2ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9", "%.3ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9", "%.4ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9", "%.5ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9\xe2\x82\xac", "%.6ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9\xe2\x82\xac", "%.7ls", mixed));
    _TEST_TRUE(prints("a\xc3\xa9", "%.*ls", 4, mixed));

    _TEST_TRUE(prints("", "%.1ls", mixed + 1));
    _TEST_TRUE(prints("", "%.2ls", mixed + 2));
    _TEST_TRUE(prints("\xe2\x82\xac", "%.3ls", mixed + 2));

    _TEST_TRUE(prints("     a", "%6.2ls", mixed));
    _TEST_TRUE(prints("a   |", "%-4.2ls|", mixed));
    _TEST_TRUE(prints("   a\xc3\xa9", "%6.5ls", mixed));

    _TEST_TRUE(prints("\xc3\xa9", "%.2ls", open));
    _TEST_TRUE(prints("\xc3\xa9\xe2\x82\xac", "%.5ls", open));
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_StdioWidePartial();
    return _Test_Status();
}
