// (Test) Status: 0
// A test file for <wchar.h> wide character output.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"
#define UTF8 "C.UTF-8"

// (S7.24.3.3) A wide character put should be the stream's next bytes.
static void Test_WcharWideOutput(void)
{
    FILE *file = fopen(NAME, "wb");

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputwc(L'a', file) == L'a');
    _TEST_TRUE(putwc(L'b', file) == L'b');
    _TEST_TRUE((fputwc)(L'c', file) == L'c');
    _TEST_TRUE((putwc)(L'\n', file) == L'\n');
    _TEST_TRUE(fputws(L"de", file) >= 0);
    _TEST_TRUE(fputws(L"", file) >= 0);
    _TEST_TRUE((fputws)(L"f\n", file) >= 0);
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds(NAME, "abc\ndef\n", 8));
    _TEST_INT(remove(NAME), 0);
}

// (S7.24.3.3) In UTF-8 a wide character put should be its sequence of bytes.
static void Test_WcharWideOutputUtf8(void)
{
    FILE *file;
    static const wchar_t text[] = {0x20AC, L'z', 0x1F600, L'\n', L'\0'};

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    file = fopen(NAME, "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputwc(L'a', file) == L'a');
    _TEST_TRUE(fputwc(0xE9, file) == 0xE9);
    _TEST_TRUE(putwc(0x20AC, file) == 0x20AC);
    _TEST_TRUE(fputwc(0x1F600, file) == 0x1F600);
    _TEST_TRUE(fputws(text, file) >= 0);
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds(NAME, "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\xE2\x82\xACz\xF0\x9F\x98\x80\n", 19));
    _TEST_INT(remove(NAME), 0);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

// (S7.24.3.3p3) A wide character put in a stream not to write should fail.
static void Test_WcharWideOutputErrors(void)
{
    FILE *file = holding(NAME, "abc", 3);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputwc(L'x', file) == WEOF);
    _TEST_TRUE(ferror(file));
    clearerr(file);
    _TEST_INT(fputws(L"xy", file), EOF);
    _TEST_TRUE(ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

int main(void)
{
    Test_WcharWideOutput();
    Test_WcharWideOutputUtf8();
    Test_WcharWideOutputErrors();
    return _Test_Status();
}
