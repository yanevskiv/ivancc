// (Test) Status: 0
// A test file for <stdio.h> the lc and ls conversions of multibyte characters.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %lc should write the bytes wctomb gives a wide character.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static bool encodes(wchar_t wc)
{
    int lost = 0;
    char got[SMALL];
    char want[MB_LEN_MAX];
    int len = wctomb(want, wc);

    if (! _TEST_TRUE(len > 0)) {
        return false;
    }
    lost += ! _TEST_INT(snprintf(got, sizeof(got), "%lc", wc), len);
    lost += ! _TEST_MEM(got, want, (size_t) len);
    lost += ! _TEST_INT(got[len], '\0');
    return lost == 0;
}

// %ls should write the bytes wcstombs gives, counted by a width.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static bool converts(const wchar_t *wide)
{
    int lost = 0;
    char got[HOLD];
    char want[LINE];
    size_t len = wcstombs(want, wide, sizeof(want));

    if (! _TEST_TRUE(len < sizeof(want))) {
        return false;
    }
    lost += ! _TEST_INT(snprintf(got, sizeof(got), "%ls", wide), (int) len);
    lost += ! _TEST_STR(got, want);
    lost += ! _TEST_INT(snprintf(got, sizeof(got), "%*ls", (int) len + 2, wide), (int) len + 2);
    lost += ! _TEST_MEM(got, "  ", 2);
    lost += ! _TEST_STR(got + 2, want);
    lost += ! _TEST_INT(snprintf(got, sizeof(got), "%-*ls|", (int) len + 2, wide), (int) len + 3);
    lost += ! _TEST_MEM(got, want, len);
    lost += ! _TEST_STR(got + len, "  |");
    return lost == 0;
}

// %lc and %ls should write wide characters as the locale's bytes.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static void Test_StdioWideMultibyte(void)
{
    const wchar_t euro[] = {0x20ac, L'\0'};
    const wchar_t mixed[] = {L'a', 0xe9, L'b', 0x20ac, 0x1f600, L'\0'};

    _TEST_TRUE(encodes(L'a'));
    _TEST_TRUE(converts(L"abc"));
    _TEST_TRUE(converts(L""));

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(encodes(L'a'));
    _TEST_TRUE(encodes(0xe9));
    _TEST_TRUE(encodes(0x20ac));
    _TEST_TRUE(encodes(0x1f600));
    _TEST_TRUE(converts(L"abc"));
    _TEST_TRUE(converts(L""));
    _TEST_TRUE(converts(euro));
    _TEST_TRUE(converts(mixed));

    _TEST_TRUE(prints("\xc3\xa9", "%lc", 0xe9));
    _TEST_TRUE(prints("\xe2\x82\xac", "%lc", 0x20ac));
    _TEST_TRUE(prints("\xe2\x82\xac", "%ls", euro));
    _TEST_TRUE(prints("a\xc3\xa9" "b\xe2\x82\xac\xf0\x9f\x98\x80", "%ls", mixed));
    _TEST_TRUE(prints("  \xe2\x82\xac", "%5lc", 0x20ac));
    _TEST_TRUE(prints("\xe2\x82\xac  |", "%-5ls|", euro));
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_StdioWideMultibyte();
    return _Test_Status();
}
