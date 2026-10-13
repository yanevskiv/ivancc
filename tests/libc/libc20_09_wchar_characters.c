// (Test) Status: 0
// A test file for <wchar.h> restartable character conversions in "C".

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:S7.24.6.2.1] mbsinit should hold of no state and of a zeroed one.
static void Test_WcharStateInitial(void)
{
    mbstate_t state;

    memset(&state, 0, sizeof(state));
    _TEST_TRUE(mbsinit(NULL) != 0);
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_TRUE((mbsinit)(&state) != 0);
}

// [C99:S7.24.6.3.2] mbrtowc should give a character and its length, 0 for a null.
static void Test_WcharDecoding(void)
{
    mbstate_t state;
    wchar_t wc = L'?';

    memset(&state, 0, sizeof(state));
    _TEST_UINT(mbrtowc(&wc, "A", 1, &state), 1);
    _TEST_INT(wc, L'A');
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_UINT(mbrtowc(&wc, "BC", 2, &state), 1);
    _TEST_INT(wc, L'B');
    _TEST_UINT(mbrtowc(&wc, "", 1, &state), 0);
    _TEST_INT(wc, L'\0');
    _TEST_TRUE(mbsinit(&state) != 0);
    wc = L'?';
    _TEST_UINT(mbrtowc(NULL, "D", 1, &state), 1);
    _TEST_INT(wc, L'?');
    _TEST_UINT(mbrtowc(&wc, NULL, 0, &state), 0);
    _TEST_INT(wc, L'?');
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_TRUE(mbrtowc(&wc, "E", 0, &state) == (size_t) -2);
    _TEST_INT(wc, L'?');
    _TEST_UINT(mbrtowc(&wc, "F", 1, NULL), 1);
    _TEST_INT(wc, L'F');
    _TEST_UINT((mbrtowc)(&wc, "G", 1, &state), 1);
    _TEST_INT(wc, L'G');
}

// [C99:S7.24.6.3.1] mbrlen should give what mbrtowc would, and store nothing.
static void Test_WcharLength(void)
{
    mbstate_t state;

    memset(&state, 0, sizeof(state));
    _TEST_UINT(mbrlen("A", 1, &state), 1);
    _TEST_UINT(mbrlen("AB", 2, &state), 1);
    _TEST_UINT(mbrlen("", 1, &state), 0);
    _TEST_TRUE(mbrlen("A", 0, &state) == (size_t) -2);
    _TEST_UINT(mbrlen("A", 1, NULL), 1);
    _TEST_UINT((mbrlen)("A", 1, &state), 1);
    _TEST_TRUE(mbsinit(&state) != 0);
}

// [C99:S7.24.6.3.3] wcrtomb should give a character's bytes, one for no buffer.
static void Test_WcharEncoding(void)
{
    mbstate_t state;
    char bytes[SIZE];

    memset(&state, 0, sizeof(state));
    memset(bytes, 'x', sizeof(bytes));
    _TEST_TRUE(MB_CUR_MAX <= SIZE);
    _TEST_UINT(wcrtomb(bytes, L'A', &state), 1);
    _TEST_MEM(bytes, "Ax", 2);
    _TEST_UINT(wcrtomb(bytes + 1, L'\0', &state), 1);
    _TEST_MEM(bytes, "A\0x", 3);
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_UINT(wcrtomb(NULL, L'B', &state), 1);
    _TEST_MEM(bytes, "A\0x", 3);
    _TEST_TRUE(mbsinit(&state) != 0);
    _TEST_UINT(wcrtomb(bytes, L'C', NULL), 1);
    _TEST_INT(bytes[0], 'C');
    _TEST_UINT((wcrtomb)(bytes, L'D', &state), 1);
    _TEST_INT(bytes[0], 'D');
}

// [C99:S7.24.6.3] A character "C" has none of should be an error, EILSEQ in errno.
static void Test_WcharCharacterErrors(void)
{
    mbstate_t state;
    char bytes[SIZE];
    wchar_t wc = L'?';

    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(wcrtomb(bytes, 0x20AC, &state) == (size_t) -1);
    _TEST_INT(errno, EILSEQ);
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrtowc(&wc, "\xE9", 1, &state) == (size_t) -1);
    _TEST_INT(errno, EILSEQ);
    _TEST_INT(wc, L'?');
    memset(&state, 0, sizeof(state));
    errno = 0;
    _TEST_TRUE(mbrlen("\xE9", 1, &state) == (size_t) -1);
    _TEST_INT(errno, EILSEQ);
}

int main(void)
{
    Test_WcharStateInitial();
    Test_WcharDecoding();
    Test_WcharLength();
    Test_WcharEncoding();
    Test_WcharCharacterErrors();
    return _Test_Status();
}
