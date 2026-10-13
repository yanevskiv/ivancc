// (Test) Status: 0
// A test file for <wchar.h> reading wide characters beyond ASCII in UTF-8.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8 "C.UTF-8"

// [C99 S7.24.2.2p12] A wide character stored as bytes should be its multibyte one.
static void Test_WcharSwscanfUtf8Bytes(void)
{
    int n = NONE;
    char out[SIZE];
    static const wchar_t input[] = {L'h', 0xE9, 0x20AC, 0x1F600, L' ', L'z', L'\0'};

    _TEST_INT(swscanf(input, L"%s%n", out, &n), 1);
    _TEST_STR(out, "h\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80");
    _TEST_INT(n, 4);
    _TEST_INT(swscanf(input, L"%3s%n", out, &n), 1);
    _TEST_STR(out, "h\xC3\xA9\xE2\x82\xAC");
    _TEST_INT(n, 3);
    memset(out, 'x', sizeof(out));
    _TEST_INT(swscanf(input, L"%2c%n", out, &n), 1);
    _TEST_MEM(out, "h\xC3\xA9x", 4);
    _TEST_INT(n, 2);
    _TEST_INT(swscanf(input, L"%[^ ]%n", out, &n), 1);
    _TEST_STR(out, "h\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80");
    _TEST_INT(n, 4);
}

// [C99 S7.24.2.2p12] A scanset should hold wide characters, and a width count them.
static void Test_WcharSwscanfUtf8Wide(void)
{
    int n = NONE;
    wchar_t out[SIZE];
    static const wchar_t listed[] = {0xE0, 0xE9, 0x20AC, L'\0'};
    static const wchar_t input[] = {0xE0, 0xE9, 0x20AC, 0xEA, L' ', L'z', L'\0'};
    static const wchar_t set[] = {L'%', L'l', L'[', 0xE0, 0xE9, 0x20AC, L']', L'%', L'n', L'\0'};
    static const wchar_t others[] = {L'%', L'l', L'[', L'^', 0x20AC, L']', L'%', L'n', L'\0'};

    _TEST_INT(swscanf(input, set, out, &n), 1);
    _TEST_MEM(out, listed, sizeof(listed));
    _TEST_INT(n, 3);
    _TEST_INT(swscanf(input, others, out, &n), 1);
    _TEST_MEM(out, listed, WIDE(2));
    _TEST_INT(out[2], 0);
    _TEST_INT(n, 2);
    _TEST_INT(swscanf(input, L"%2ls%n", out, &n), 1);
    _TEST_MEM(out, listed, WIDE(2));
    _TEST_INT(out[2], 0);
    _TEST_INT(n, 2);
    _TEST_INT(swscanf(input, L"%3lc%n", out, &n), 1);
    _TEST_MEM(out, listed, WIDE(3));
    _TEST_INT(n, 3);
}

// [C99 S7.24.2.2p8] White space should be what iswspace says it is.
static void Test_WcharSwscanfUtf8Space(void)
{
    int i = 0;
    int j = 0;
    wchar_t out[SIZE];
    static const wchar_t word[] = {L'a', L'b', 0x2003, L'c', L'\0'};
    static const wchar_t format[] = {L'%', L'd', 0x2003, L'%', L'd', L'\0'};
    static const wchar_t input[] = {0x2003, L'1', 0x2003, 0x3000, L'2', L'\0'};

    if (! _TEST_TRUE(iswspace(0x2003) && iswspace(0x3000))) {
        return;
    }
    _TEST_INT(swscanf(input, L"%d%d", &i, &j), 2);
    _TEST_INT(i, 1);
    _TEST_INT(j, 2);
    _TEST_INT(swscanf(L"3 4", format, &i, &j), 2);
    _TEST_INT(i, 3);
    _TEST_INT(j, 4);
    _TEST_INT(swscanf(word, L"%ls", out), 1);
    _TEST_MEM(out, L"ab", WIDE(3));
}

int main(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return _Test_Status();
    }
    Test_WcharSwscanfUtf8Bytes();
    Test_WcharSwscanfUtf8Wide();
    Test_WcharSwscanfUtf8Space();
    return _Test_Status();
}
