// (Test) Status: 0
// A test file for <inttypes.h> calls by pointer and past a macro.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99/S7.1.4p1] A pointer and a call in parentheses should reach each function.
static void Test_InttypesCalls(void)
{
    intmax_t (*absolute)(intmax_t) = imaxabs;
    imaxdiv_t (*divide)(intmax_t, intmax_t) = imaxdiv;
    intmax_t (*narrow_signed)(const char *restrict, char **restrict, int) = strtoimax;
    uintmax_t (*narrow_unsigned)(const char *restrict, char **restrict, int) = strtoumax;
    intmax_t (*wide_signed)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoimax;
    uintmax_t (*wide_unsigned)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoumax;

    _TEST_INT(absolute(-3), 3);
    _TEST_INT(divide(9, 4).quot, 2);
    _TEST_INT(divide(9, 4).rem, 1);
    _TEST_INT(narrow_signed("-42", NULL, 10), -42);
    _TEST_UINT(narrow_unsigned("42", NULL, 10), 42);
    _TEST_INT(wide_signed(L"-42", NULL, 10), -42);
    _TEST_UINT(wide_unsigned(L"42", NULL, 10), 42);

    _TEST_INT((imaxabs)(-3), 3);
    _TEST_INT((imaxdiv)(9, 4).quot, 2);
    _TEST_INT((imaxdiv)(9, 4).rem, 1);
    _TEST_INT((strtoimax)("-42", NULL, 10), -42);
    _TEST_UINT((strtoumax)("42", NULL, 10), 42);
    _TEST_INT((wcstoimax)(L"-42", NULL, 10), -42);
    _TEST_UINT((wcstoumax)(L"42", NULL, 10), 42);
}

int main(void)
{
    Test_InttypesCalls();
    return _Test_Status();
}
