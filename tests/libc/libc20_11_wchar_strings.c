// (Test) Status: 0
// A test file for <wchar.h> restartable string conversions in "C".

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define BAD ((size_t) -1)

// [C99:7.24.6.4.1] mbsrtowcs should convert up to len, and leave *src past them.
static void Test_WcharStringsDecoding(void)
{
    mbstate_t state;
    const char *src;
    wchar_t wide[SIZE];
    const char *text = "hello";

    memset(&state, 0, sizeof(state));
    put(wide, L"xxxxxxxx", FILLED);
    src = text;
    _TEST_UINT(mbsrtowcs(wide, &src, SIZE, &state), 5);
    _TEST_MEM(wide, L"hello\0xx", WIDE(8));
    _TEST_TRUE(src == NULL);
    _TEST_TRUE(mbsinit(&state) != 0);

    put(wide, L"xxxxxxxx", FILLED);
    src = text;
    _TEST_UINT(mbsrtowcs(wide, &src, 3, &state), 3);
    _TEST_MEM(wide, L"helxxxxx", WIDE(8));
    _TEST_TRUE(src == text + 3);
    _TEST_UINT(mbsrtowcs(wide + 3, &src, 2, &state), 2);
    _TEST_MEM(wide, L"helloxxx", WIDE(8));
    _TEST_TRUE(src == text + 5);
    _TEST_UINT(mbsrtowcs(wide + 5, &src, 1, &state), 0);
    _TEST_MEM(wide, L"hello\0xx", WIDE(8));
    _TEST_TRUE(src == NULL);

    src = text;
    _TEST_UINT(mbsrtowcs(wide, &src, 0, &state), 0);
    _TEST_TRUE(src == text);
    _TEST_UINT(mbsrtowcs(NULL, &src, 0, &state), 5);
    _TEST_TRUE(src == text);
    _TEST_UINT(mbsrtowcs(NULL, &src, 2, &state), 5);
    _TEST_TRUE(src == text);
    src = "";
    _TEST_UINT(mbsrtowcs(wide, &src, SIZE, NULL), 0);
    _TEST_TRUE(src == NULL);
    _TEST_INT(wide[0], L'\0');
    src = text;
    _TEST_UINT((mbsrtowcs)(wide, &src, SIZE, NULL), 5);
    _TEST_TRUE(src == NULL);
}

// [C99:7.24.6.4.2] wcsrtombs should convert up to len, and leave *src past them.
static void Test_WcharStringsEncoding(void)
{
    mbstate_t state;
    const wchar_t *src;
    char bytes[SIZE];
    const wchar_t *text = L"hello";

    memset(&state, 0, sizeof(state));
    memset(bytes, 'x', sizeof(bytes));
    src = text;
    _TEST_UINT(wcsrtombs(bytes, &src, SIZE, &state), 5);
    _TEST_MEM(bytes, "hello\0xx", 8);
    _TEST_TRUE(src == NULL);
    _TEST_TRUE(mbsinit(&state) != 0);

    memset(bytes, 'x', sizeof(bytes));
    src = text;
    _TEST_UINT(wcsrtombs(bytes, &src, 3, &state), 3);
    _TEST_MEM(bytes, "helxxxxx", 8);
    _TEST_TRUE(src == text + 3);
    _TEST_UINT(wcsrtombs(bytes + 3, &src, 2, &state), 2);
    _TEST_MEM(bytes, "helloxxx", 8);
    _TEST_TRUE(src == text + 5);
    _TEST_UINT(wcsrtombs(bytes + 5, &src, 1, &state), 0);
    _TEST_MEM(bytes, "hello\0xx", 8);
    _TEST_TRUE(src == NULL);

    src = text;
    _TEST_UINT(wcsrtombs(bytes, &src, 0, &state), 0);
    _TEST_TRUE(src == text);
    _TEST_UINT(wcsrtombs(NULL, &src, 0, &state), 5);
    _TEST_TRUE(src == text);
    _TEST_UINT(wcsrtombs(NULL, &src, 2, &state), 5);
    _TEST_TRUE(src == text);
    src = L"";
    _TEST_UINT(wcsrtombs(bytes, &src, SIZE, NULL), 0);
    _TEST_TRUE(src == NULL);
    _TEST_INT(bytes[0], '\0');
    src = text;
    _TEST_UINT((wcsrtombs)(bytes, &src, SIZE, NULL), 5);
    _TEST_TRUE(src == NULL);
}

// [C99:7.24.6.4] A character "C" has none of should be an error, EILSEQ in errno.
static void Test_WcharStringsErrors(void)
{
    mbstate_t state;
    const char *src = "ab\xE9" "c";
    const wchar_t *wsrc = L"ab\x20AC" L"c";
    char bytes[SIZE];
    wchar_t wide[SIZE];

    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbsrtowcs(wide, &src, SIZE, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(wcsrtombs(bytes, &wsrc, SIZE, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(wcsrtombs(NULL, &wsrc, 0, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
}

int main(void)
{
    Test_WcharStringsDecoding();
    Test_WcharStringsEncoding();
    Test_WcharStringsErrors();
    return _Test_Status();
}
