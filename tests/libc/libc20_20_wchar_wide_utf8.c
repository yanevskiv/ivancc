// (Test) Status: 0
// A test file for <wchar.h> wide character input in UTF-8.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"
#define UTF8 "C.UTF-8"

// [C99 S7.24.3.1] In UTF-8 a wide character got should be a sequence of bytes.
static void Test_WcharWideUtf8Input(void)
{
    wchar_t line[SIZE];
    static const wchar_t first[] = {L'a', 0xE9, 0x20AC, L'\n', L'\0'};
    static const wchar_t second[] = {0x1F600, L'z', L'\0'};
    FILE *file = holding(NAME, "a\xC3\xA9\xE2\x82\xAC\n\xF0\x9F\x98\x80z", 12);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(fgetwc(file) == 0xE9);
    _TEST_TRUE(getwc(file) == 0x20AC);
    _TEST_TRUE(fgetwc(file) == L'\n');
    _TEST_TRUE(fgetwc(file) == 0x1F600);
    _TEST_TRUE(fgetwc(file) == L'z');
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(! ferror(file));

    rewind(file);
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, first, sizeof(first));
    _TEST_TRUE(fgetws(line, 2, file) == line);
    _TEST_MEM(line, second, WIDE(1));
    _TEST_INT(line[1], L'\0');
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, second + 1, WIDE(2));
    _TEST_INT(fclose(file), 0);
}

// [C99 S7.24.3.10] ungetwc should give back a wide character of several bytes.
static void Test_WcharWideUtf8Pushback(void)
{
    FILE *file = holding(NAME, "a\xE2\x82\xACz", 5);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(ungetwc(0x1F600, file) == 0x1F600);
    _TEST_TRUE(fgetwc(file) == 0x1F600);
    _TEST_TRUE(fgetwc(file) == 0x20AC);
    _TEST_TRUE(ungetwc(0x20AC, file) == 0x20AC);
    _TEST_TRUE(fgetwc(file) == 0x20AC);
    _TEST_TRUE(fgetwc(file) == L'z');
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_TRUE(ungetwc(0xE9, file) == 0xE9);
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(fgetwc(file) == 0xE9);
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_INT(fclose(file), 0);
}

// [C99 S7.24.3.1p3] Bytes that are no character should be an error, EILSEQ.
static void Test_WcharWideUtf8Errors(void)
{
    wchar_t line[SIZE];
    FILE *file = holding(NAME, "a\xFFz", 3);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    errno = 0;
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_INT(errno, EILSEQ);
    _TEST_TRUE(! feof(file));
    _TEST_INT(fclose(file), 0);

    file = holding(NAME, "ab\xE2" "A\n", 5);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    errno = 0;
    _TEST_TRUE(fgetws(line, SIZE, file) == NULL);
    _TEST_INT(errno, EILSEQ);
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

int main(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return _Test_Status();
    }
    Test_WcharWideUtf8Input();
    Test_WcharWideUtf8Pushback();
    Test_WcharWideUtf8Errors();
    return _Test_Status();
}
