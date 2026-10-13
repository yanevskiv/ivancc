// (Test) Status: 0
// A test file for <wchar.h> reading a stream by a wide format.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"
#define UTF8 "C.UTF-8"

// (S7.24.2.2p2) fwscanf should read a stream, which it orients wide.
static void Test_WcharFwscanf(void)
{
    int i = 0;
    int j = 0;
    char word[SIZE];
    wchar_t wide[SIZE];
    FILE *file = holding(NAME, "12 word 34wide x", 16);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwide(file, 0), 0);
    _TEST_INT(fwscanf(file, L"%d %s", &i, word), 2);
    _TEST_TRUE(fwide(file, 0) > 0);
    _TEST_INT(i, 12);
    _TEST_STR(word, "word");
    _TEST_INT((fwscanf)(file, L"%d%ls", &j, wide), 2);
    _TEST_INT(j, 34);
    _TEST_MEM(wide, L"wide", WIDE(5));
    _TEST_INT(fwscanf(file, L"%d", &i), 0);
    _TEST_TRUE(fgetwc(file) == L'x');
    _TEST_INT(fwscanf(file, L"%d", &i), EOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// (S7.24.2.2p9) The wide character after an input item should remain unread.
static void Test_WcharFwscanfUnread(void)
{
    int i = 0;
    int j = 0;
    wchar_t wide[SIZE];
    FILE *file = holding(NAME, "56,78\n9", 7);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwscanf(file, L"%d", &i), 1);
    _TEST_INT(i, 56);
    _TEST_TRUE(fgetwc(file) == L',');
    _TEST_TRUE(ungetwc(L'-', file) == L'-');
    _TEST_INT(fwscanf(file, L"%d%lc", &i, wide), 2);
    _TEST_INT(i, -78);
    _TEST_TRUE(wide[0] == L'\n');
    _TEST_INT(fwscanf(file, L"%d%d", &i, &j), 1);
    _TEST_INT(i, 9);
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// (S7.24.2.2p9) In UTF-8 a stream's bytes should be read as wide characters.
static void Test_WcharFwscanfUtf8(void)
{
    int i = 0;
    FILE *file;
    int n = NONE;
    char word[SIZE];
    wchar_t wide[SIZE];
    static const wchar_t first[] = {0xE9, 0x1F600, L'\0'};
    static const wchar_t format[] = {L'%', L'd', 0x20AC, L'%', L'2', L'l', L's', L'%', L'n', L'\0'};

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    file = holding(NAME, "7\xE2\x82\xAC\xC3\xA9\xF0\x9F\x98\x80\xC3\xA8z \xE2\x82\xAC", 17);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwscanf(file, format, &i, wide, &n), 2);
    _TEST_INT(i, 7);
    _TEST_MEM(wide, first, sizeof(first));
    _TEST_INT(n, 4);
    _TEST_INT(fwscanf(file, L"%s", word), 1);
    _TEST_STR(word, "\xC3\xA8z");
    _TEST_INT(fwscanf(file, L"%d", &i), 0);
    _TEST_TRUE(fgetwc(file) == 0x20AC);
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_WcharFwscanf();
    Test_WcharFwscanfUnread();
    Test_WcharFwscanfUtf8();
    return _Test_Status();
}
