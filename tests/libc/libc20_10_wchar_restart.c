// (Test) Status: 0
// A test file for <wchar.h> restartable character conversions in UTF-8.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8 "C.UTF-8"
#define CUT  ((size_t) -2)
#define BAD  ((size_t) -1)

// [C99 S7.24.6.3.2p4] mbrtowc should take a character whole, or in parts by *ps.
static void Test_WcharRestart(void)
{
    mbstate_t state;
    wchar_t wc = L'?';

    memset(&state, 0, sizeof(state));
    _TEST_UINT(mbrtowc(&wc, "\xC3\xA9", 2, &state), 2);
    _TEST_INT(wc, 0xE9);
    _TEST_UINT(mbrtowc(&wc, "\xE2\x82\xACxyz", 6, &state), 3);
    _TEST_INT(wc, 0x20AC);
    _TEST_UINT(mbrtowc(&wc, "\xF0\x9F\x98\x80", 4, &state), 4);
    _TEST_INT(wc, 0x1F600);
    _TEST_TRUE(mbsinit(&state) != 0);

    wc = L'?';
    _TEST_TRUE(mbrtowc(&wc, "\xE2", 1, &state) == CUT);
    _TEST_TRUE(mbsinit(&state) == 0);
    _TEST_TRUE(mbrtowc(&wc, "\x82", 1, &state) == CUT);
    _TEST_TRUE(mbsinit(&state) == 0);
    _TEST_INT(wc, L'?');
    _TEST_TRUE(mbrtowc(&wc, "", 0, &state) == CUT);
    _TEST_TRUE(mbsinit(&state) == 0);
    _TEST_UINT(mbrtowc(&wc, "\xACxyz", 4, &state), 1);
    _TEST_INT(wc, 0x20AC);
    _TEST_TRUE(mbsinit(&state) != 0);

    _TEST_TRUE(mbrtowc(&wc, "\xF0\x9F", 2, &state) == CUT);
    _TEST_UINT(mbrtowc(&wc, "\x98\x80z", 3, &state), 2);
    _TEST_INT(wc, 0x1F600);
    _TEST_TRUE(mbrtowc(&wc, "\xC3", 1, &state) == CUT);
    _TEST_UINT(mbrtowc(NULL, "\xA9", 1, &state), 1);
    _TEST_INT(wc, 0x1F600);
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_UINT(mbrtowc(&wc, "", 1, &state), 0);
    _TEST_INT(wc, L'\0');
}

// [C99 S7.24.6.3.1] mbrlen should measure a character in parts, by *ps as well.
static void Test_WcharRestartLength(void)
{
    mbstate_t state;

    memset(&state, 0, sizeof(state));
    _TEST_UINT(mbrlen("\xE2\x82\xAC", 3, &state), 3);
    _TEST_UINT(mbrlen("\xE2\x82\xAC", 8, &state), 3);
    _TEST_TRUE(mbrlen("\xE2\x82\xAC", 2, &state) == CUT);
    _TEST_TRUE(mbsinit(&state) == 0);
    _TEST_UINT(mbrlen("\xAC", 1, &state), 1);
    _TEST_TRUE(mbsinit(&state) != 0);
}

// [C99 S7.24.6.3p1] With no *ps each function should keep a state of its own.
static void Test_WcharRestartOwn(void)
{
    wchar_t wc = L'?';

    _TEST_TRUE(mbrtowc(&wc, "\xE2\x82", 2, NULL) == CUT);
    _TEST_TRUE(mbrlen("\xC3", 1, NULL) == CUT);
    _TEST_UINT(mbrtowc(&wc, "\xAC", 1, NULL), 1);
    _TEST_INT(wc, 0x20AC);
    _TEST_UINT(mbrlen("\xA9", 1, NULL), 1);
    _TEST_UINT(mbrlen("a", 1, NULL), 1);
    _TEST_UINT(mbrtowc(&wc, "b", 1, NULL), 1);
    _TEST_INT(wc, L'b');
}

// [C99 S7.24.6.3.3] wcrtomb should give a character's bytes, MB_CUR_MAX at most.
static void Test_WcharRestartEncoding(void)
{
    mbstate_t state;
    char bytes[SIZE];

    memset(&state, 0, sizeof(state));
    memset(bytes, 'x', sizeof(bytes));
    _TEST_UINT(wcrtomb(bytes, 0xE9, &state), 2);
    _TEST_MEM(bytes, "\xC3\xA9x", 3);
    _TEST_UINT(wcrtomb(bytes, 0x20AC, &state), 3);
    _TEST_MEM(bytes, "\xE2\x82\xACx", 4);
    _TEST_UINT(wcrtomb(bytes, 0x1F600, &state), 4);
    _TEST_MEM(bytes, "\xF0\x9F\x98\x80x", 5);
    _TEST_TRUE(wcrtomb(bytes, 0x10FFFF, NULL) <= MB_CUR_MAX);
    _TEST_UINT(wcrtomb(bytes, L'\0', &state), 1);
    _TEST_INT(bytes[0], '\0');
    _TEST_UINT(wcrtomb(NULL, 0x20AC, &state), 1);
    _TEST_TRUE(mbsinit(&state) != 0);
}

// [C99 S7.24.6.3] What is no character should be an error, EILSEQ in errno.
static void Test_WcharRestartErrors(void)
{
    mbstate_t state;
    char bytes[SIZE];
    wchar_t wc = L'?';

    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrtowc(&wc, "\xFF", 1, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrtowc(&wc, "\x82", 1, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrtowc(&wc, "\xE2" "A", 2, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrtowc(&wc, "\xE2", 1, &state) == CUT);
    _TEST_INT(errno, 0);
    _TEST_TRUE(mbrtowc(&wc, "A", 1, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    _TEST_INT(wc, L'?');
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrlen("\xC3\xC3", 2, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(wcrtomb(bytes, 0xD800, &state) == BAD);
    _TEST_INT(errno, EILSEQ);
}

int main(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return _Test_Status();
    }
    Test_WcharRestart();
    Test_WcharRestartLength();
    Test_WcharRestartOwn();
    Test_WcharRestartEncoding();
    Test_WcharRestartErrors();
    return _Test_Status();
}
