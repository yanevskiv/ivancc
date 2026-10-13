// (Test) Status: 0
// A test file for <wchar.h> formatting to a stream.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"
#define UTF8 "C.UTF-8"

// [C99:S7.24.2.1] fwprintf should write to a stream, which it orients wide.
static void Test_WcharFwprintf(void)
{
    FILE *file = fopen(NAME, "wb");

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwide(file, 0), 0);
    _TEST_INT(fwprintf(file, L"abc %d|%5s|%-3lc|\n", 12, "xy", (wint_t) L'z'), 18);
    _TEST_TRUE(fwide(file, 0) > 0);
    _TEST_INT((fwprintf)(file, L""), 0);
    _TEST_INT(fwprintf(file, L"%ls%%\n", L"wide"), 6);
    _TEST_TRUE(fputwc(L'!', file) == L'!');
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    holds(NAME, "abc 12|   xy|z  |\nwide%\n!", 25);

    file = fopen(NAME, "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fwprintf(file, L"abc") < 0);
    _TEST_TRUE(ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// [C99:S7.24.2.1p14] In UTF-8 a wide character written should be its bytes.
static void Test_WcharFwprintfUtf8(void)
{
    FILE *file;
    static const wchar_t wide[] = {L'h', 0xE9, L'\0'};
    static const wchar_t format[] = {L'a', 0x20AC, L'%', L'l', L's', L'%', L'l', L'c', L'%', L's', L'|', L'%', L'3', L'l', L's', L'|', L'\0'};

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    file = fopen(NAME, "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwprintf(file, format, wide, (wint_t) 0x1F600, "\xC3\xA9z", wide), 12);
    _TEST_INT(fclose(file), 0);
    holds(NAME, "a\xE2\x82\xACh\xC3\xA9\xF0\x9F\x98\x80\xC3\xA9z| h\xC3\xA9|", 20);
    _TEST_INT(remove(NAME), 0);
}

int main(void)
{
    Test_WcharFwprintf();
    Test_WcharFwprintfUtf8();
    return _Test_Status();
}
