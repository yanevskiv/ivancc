// (Test) Status: 0
// A test file for <wchar.h> restartable string conversions in UTF-8.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8 "C.UTF-8"
#define CUT  ((size_t) -2)
#define BAD  ((size_t) -1)

// mbsrtowcs should count wide characters, and stop between two.
// [C99:7.24.6.4.1]: mbsrtowcs
static void Test_WcharStringsUtf8Decoding(void)
{
    mbstate_t state;
    const char *src;
    wchar_t wide[SIZE];
    const char *text = "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80z";
    static const wchar_t same[] = {L'a', 0xE9, 0x20AC, 0x1F600, L'z', L'\0', L'x'};

    memset(&state, 0, sizeof(state));
    put(wide, L"xxxxxxxx", FILLED);
    src = text;
    _TEST_UINT(mbsrtowcs(wide, &src, SIZE, &state), 5);
    _TEST_MEM(wide, same, WIDE(7));
    _TEST_TRUE(src == NULL);
    _TEST_TRUE(mbsinit(&state) != 0);

    put(wide, L"xxxxxxxx", FILLED);
    src = text;
    _TEST_UINT(mbsrtowcs(wide, &src, 3, &state), 3);
    _TEST_MEM(wide, same, WIDE(3));
    _TEST_INT(wide[3], L'x');
    _TEST_TRUE(src == text + 6);
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_UINT(mbsrtowcs(wide + 3, &src, SIZE, &state), 2);
    _TEST_MEM(wide, same, WIDE(6));
    _TEST_TRUE(src == NULL);
    src = text;
    _TEST_UINT(mbsrtowcs(NULL, &src, 0, &state), 5);
    _TEST_TRUE(src == text);
}

// mbsrtowcs should begin in the state *ps describes.
// [C99:7.24.6.4.1p2]: mbsrtowcs, stopping at a null character or after len
static void Test_WcharStringsUtf8Begun(void)
{
    mbstate_t state;
    wchar_t wide[SIZE];
    const char *src = "\x82\xAC" "b";

    memset(&state, 0, sizeof(state));
    _TEST_TRUE(mbrtowc(wide, "\xE2", 1, &state) == CUT);
    _TEST_UINT(mbsrtowcs(wide, &src, SIZE, &state), 2);
    _TEST_INT(wide[0], 0x20AC);
    _TEST_INT(wide[1], L'b');
    _TEST_INT(wide[2], L'\0');
    _TEST_TRUE(src == NULL);
    _TEST_TRUE(mbsinit(&state) != 0);
}

// wcsrtombs should count bytes, and stop between two characters.
// [C99:7.24.6.4.2]: wcsrtombs
static void Test_WcharStringsUtf8Encoding(void)
{
    mbstate_t state;
    const wchar_t *src;
    char bytes[SIZE];
    static const wchar_t text[] = {L'a', 0xE9, 0x20AC, 0x1F600, L'z', L'\0'};
    const char *same = "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80z\0x";

    memset(&state, 0, sizeof(state));
    memset(bytes, 'x', sizeof(bytes));
    src = text;
    _TEST_UINT(wcsrtombs(bytes, &src, SIZE, &state), 11);
    _TEST_MEM(bytes, same, 13);
    _TEST_TRUE(src == NULL);
    _TEST_TRUE(mbsinit(&state) != 0);

    memset(bytes, 'x', sizeof(bytes));
    src = text;
    _TEST_UINT(wcsrtombs(bytes, &src, 5, &state), 3);
    _TEST_MEM(bytes, "a\xC3\xA9xx", 5);
    _TEST_TRUE(src == text + 2);
    _TEST_UINT(wcsrtombs(bytes + 3, &src, 3, &state), 3);
    _TEST_MEM(bytes, "a\xC3\xA9\xE2\x82\xACx", 7);
    _TEST_TRUE(src == text + 3);
    _TEST_UINT(wcsrtombs(bytes + 6, &src, 5, &state), 5);
    _TEST_MEM(bytes, "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80zx", 12);
    _TEST_TRUE(src == text + 5);
    _TEST_UINT(wcsrtombs(bytes + 11, &src, 1, &state), 0);
    _TEST_MEM(bytes, same, 13);
    _TEST_TRUE(src == NULL);
    src = text;
    _TEST_UINT(wcsrtombs(NULL, &src, 0, &state), 11);
    _TEST_TRUE(src == text);
}

// What is no character should be an error, EILSEQ in errno.
// [C99:7.24.6.4]: mbsrtowcs and wcsrtombs
static void Test_WcharStringsUtf8Errors(void)
{
    mbstate_t state;
    const char *src = "ab\xFF" "c";
    const char *cut = "ab\xE2\x82";
    char bytes[SIZE];
    wchar_t wide[SIZE];
    const wchar_t *wsrc;
    static const wchar_t none[] = {L'a', L'b', 0xD800, L'c', L'\0'};

    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbsrtowcs(wide, &src, SIZE, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbsrtowcs(wide, &cut, SIZE, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    wsrc = none;
    _TEST_TRUE(wcsrtombs(bytes, &wsrc, SIZE, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
}

int main(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return _Test_Status();
    }
    Test_WcharStringsUtf8Decoding();
    Test_WcharStringsUtf8Begun();
    Test_WcharStringsUtf8Encoding();
    Test_WcharStringsUtf8Errors();
    return _Test_Status();
}
