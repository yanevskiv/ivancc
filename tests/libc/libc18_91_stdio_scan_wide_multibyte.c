// (Test) Status: 0
// A test file for <stdio.h> scanf's ls and l[ of multibyte characters.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define MIXED "a\xc3\xa9" "b\xe2\x82\xac\xf0\x9f\x98\x80"

// [C99:S7.19.6.2p12] %ls and %l[ should give the wide characters mbstowcs gives.
static bool decodes(const char *str)
{
    int lost = 0;
    int n = NONE;
    wchar_t got[LINE];
    wchar_t want[LINE];
    size_t len = mbstowcs(want, str, LINE);

    if (! _TEST_TRUE(len < LINE)) {
        return false;
    }
    lost += ! _TEST_INT(sscanf(str, "%ls%n", got, &n), 1);
    lost += ! _TEST_MEM(got, want, (len + 1) * sizeof(wchar_t));
    lost += ! _TEST_INT(n, (int) strlen(str));
    lost += ! _TEST_INT(sscanf(str, "%l[^ ]%n", got, &n), 1);
    lost += ! _TEST_MEM(got, want, (len + 1) * sizeof(wchar_t));
    lost += ! _TEST_INT(n, (int) strlen(str));
    return lost == 0;
}

// [C99:S7.19.6.2p12] %ls and %l[ should read the locale's multibyte characters.
static void Test_StdioScanWideMultibyte(void)
{
    FILE *file;
    char ch = FILL;
    wchar_t first[SMALL];
    wchar_t second[SMALL];
    const wchar_t accent[] = {L'h', 0xe9, L'\0'};
    const wchar_t euro[] = {0x20ac, L'\0'};

    _TEST_TRUE(decodes("abc"));
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(decodes("abc"));
    _TEST_TRUE(decodes("\xc3\xa9"));
    _TEST_TRUE(decodes("\xe2\x82\xac"));
    _TEST_TRUE(decodes("\xf0\x9f\x98\x80"));
    _TEST_TRUE(decodes(MIXED));
    _TEST_INT(sscanf("h\xc3\xa9 \xe2\x82\xac!", "%ls%ls", first, second), 2);
    _TEST_MEM(first, accent, sizeof(accent));
    _TEST_INT(second[0], 0x20ac);
    _TEST_INT(second[1], L'!');
    _TEST_INT(second[2], L'\0');
    _TEST_INT(sscanf("h\xc3\xa9llo", "%l[^l]%c", first, &ch), 2);
    _TEST_MEM(first, accent, sizeof(accent));
    _TEST_INT(ch, 'l');

    // A stream's characters should decode the same, whatever its buffer cuts.
    if (put("file", "w", "  \xe2\x82\xac h\xc3\xa9\n")) {
        file = fopen("file", "r");
        if (_TEST_TRUE(file != NULL)) {
            _TEST_INT(setvbuf(file, NULL, _IONBF, 0), 0);
            _TEST_INT(fscanf(file, "%ls", first), 1);
            _TEST_MEM(first, euro, sizeof(euro));
            _TEST_INT(fscanf(file, " %l[^\n]", first), 1);
            _TEST_MEM(first, accent, sizeof(accent));
            _TEST_INT(fgetc(file), '\n');
            _TEST_INT(fscanf(file, "%ls", first), EOF);
            _TEST_INT(fclose(file), 0);
        }
    }
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_StdioScanWideMultibyte();
    return _Test_Status();
}
