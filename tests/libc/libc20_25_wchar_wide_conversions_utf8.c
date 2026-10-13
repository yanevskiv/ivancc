// (Test) Status: 0
// A test file for <wchar.h> formatting multibyte strings in UTF-8.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8 "C.UTF-8"

// (S7.24.2.1p8) %s should write the wide characters a string's bytes are.
static void Test_WcharWideUtf8String(void)
{
    int n = NONE;
    wchar_t out[SIZE];
    static const wchar_t whole[] = {L'h', 0xE9, 0x20AC, 0x1F600, L'|', L'\0'};
    static const wchar_t padded[] = {L' ', L' ', 0xE9, 0x20AC, L'|', 0xE9, L' ', L' ', L'|', L'\0'};
    static const wchar_t cut[] = {L'h', 0xE9, L'|', L'h', 0xE9, 0x20AC, L'|', L'\0'};

    _TEST_INT(swprintf(out, SIZE, L"%s|", "h\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80"), 5);
    _TEST_MEM(out, whole, sizeof(whole));
    _TEST_INT(swprintf(out, SIZE, L"%4s|%-3s|", "\xC3\xA9\xE2\x82\xAC", "\xC3\xA9"), 9);
    _TEST_MEM(out, padded, sizeof(padded));
    _TEST_INT(swprintf(out, SIZE, L"%.2s|%.3s|", "h\xC3\xA9\xE2\x82\xAC", "h\xC3\xA9\xE2\x82\xAC"), 7);
    _TEST_MEM(out, cut, sizeof(cut));
    _TEST_INT(swprintf(out, SIZE, L"%s%n", "\xC3\xA9\xE2\x82\xAC", &n), 2);
    _TEST_INT(n, 2);
}

// (S7.24.2.1p14) Bytes that are no character should fail a conversion.
static void Test_WcharWideUtf8Errors(void)
{
    wchar_t out[SIZE];

    errno = 0;
    _TEST_TRUE(swprintf(out, SIZE, L"a%s", "x\xFFy") < 0);
    _TEST_INT(errno, EILSEQ);
    errno = 0;
    _TEST_TRUE(swprintf(out, SIZE, L"a%s", "x\xC3(") < 0);
    _TEST_INT(errno, EILSEQ);
}

int main(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return _Test_Status();
    }
    Test_WcharWideUtf8String();
    Test_WcharWideUtf8Errors();
    return _Test_Status();
}
