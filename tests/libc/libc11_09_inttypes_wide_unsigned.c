// (Test) Status: 0
// A test file for <inttypes.h> wcstoumax.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99:7.8.2.4p2] wcstoumax should give the number, the end and the error given.
static bool wide_to_unsigned(const wchar_t *str, int base, uintmax_t want, long used, int error)
{
    int lost = 0;
    int number;
    wchar_t *end = NULL;
    uintmax_t got;

    errno = 0;
    got = wcstoumax(str, &end, base);
    number = errno;
    lost += ! _TEST_UINT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// [C99:7.8.2.4p2] wcstoumax should take what strtoumax does.
static void Test_InttypesWideUnsigned(void)
{
    _TEST_TRUE(wide_to_unsigned(L"-1", 10, UINTMAX_MAX, 2, 0));
    _TEST_TRUE(wide_to_unsigned(L"zz", 36, 1295, 2, 0));
    _TEST_TRUE(wide_to_unsigned(L"18446744073709551616", 10, UINTMAX_MAX, 20, ERANGE));
    _TEST_TRUE(wide_to_unsigned(L"\t0X10", 0, 16, 5, 0));
    _TEST_TRUE(wide_to_unsigned(L"", 10, 0, 0, 0));
    _TEST_UINT(wcstoumax(L"12", NULL, 10), 12);
}

int main(void)
{
    Test_InttypesWideUnsigned();
    return _Test_Status();
}
