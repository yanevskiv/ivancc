// (Test) Status: 0
// A test file for <wchar.h> types and macros.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// (S7.24.1p3) The limits and WEOF should be constants a declaration can use.
enum { LIMITS = WCHAR_MIN < WCHAR_MAX, CONSTANT = WEOF != 0 };

// (S7.24.1p3) wchar_t's limits should be its least and greatest values.
static void Test_WcharLimits(void)
{
    wchar_t least = WCHAR_MIN;
    wchar_t most = WCHAR_MAX;

    _TEST_TRUE(LIMITS);
    _TEST_TRUE(least == WCHAR_MIN);
    _TEST_TRUE(most == WCHAR_MAX);
    _TEST_TRUE(least < most);
    _TEST_TRUE(WCHAR_MIN <= 0);
    _TEST_TRUE(WCHAR_MAX >= 127);
    _TEST_TRUE(L'a' >= WCHAR_MIN && L'a' <= WCHAR_MAX);
}

// (S7.24.1p2) wint_t should hold WEOF and any wide character, told apart.
static void Test_WcharWint(void)
{
    wint_t wc = WEOF;

    _TEST_TRUE(CONSTANT);
    _TEST_TRUE(wc == WEOF);
    _TEST_UINT(sizeof(WEOF), sizeof(wint_t));
    wc = L'a';
    _TEST_TRUE(wc != WEOF);
    wc = (wint_t) WCHAR_MAX;
    _TEST_TRUE(wc != WEOF);
    wc = (wint_t) WCHAR_MIN;
    _TEST_TRUE(wc != WEOF);
}

// (S7.24.1p2) The header should give NULL, size_t, mbstate_t and struct tm.
static void Test_WcharTypes(void)
{
    wchar_t *none = NULL;
    struct tm *when = NULL;
    size_t size = sizeof(mbstate_t);
    mbstate_t state;
    mbstate_t copy;

    _TEST_TRUE(none == NULL);
    _TEST_TRUE(when == NULL);
    _TEST_TRUE(size > 0);
    memset(&state, 0, sizeof(state));
    copy = state;
    _TEST_MEM(&copy, &state, sizeof(state));
}

int main(void)
{
    Test_WcharLimits();
    Test_WcharWint();
    Test_WcharTypes();
    return _Test_Status();
}
