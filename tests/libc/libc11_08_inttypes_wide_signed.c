// (Test) Status: 0
// A test file for <inttypes.h> wcstoimax.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99:7.8.2.4p2] wcstoimax should give the number, the end and the error given.
static bool wide_to_signed(const wchar_t *str, int base, intmax_t want, long used, int error)
{
    int lost = 0;
    int number;
    intmax_t got;
    wchar_t *end = NULL;

    errno = 0;
    got = wcstoimax(str, &end, base);
    number = errno;
    lost += ! _TEST_INT(got, want);
    lost += ! _TEST_INT(end - str, used);
    lost += ! _TEST_INT(number, error);
    return lost == 0;
}

// [C99:7.8.2.4p2] wcstoimax should take what strtoimax does, in ASCII alone.
static void Test_InttypesWideSigned(void)
{
    _TEST_TRUE(wide_to_signed(L" -0x1Fz", 0, -31, 6, 0));
    _TEST_TRUE(wide_to_signed(L"+", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"0xg", 16, 0, 1, 0));
    _TEST_TRUE(wide_to_signed(L"0755", 0, 493, 4, 0));
    _TEST_TRUE(wide_to_signed(L"9223372036854775808", 10, INTMAX_MAX, 19, ERANGE));
    _TEST_TRUE(wide_to_signed(L"-9223372036854775808", 10, INTMAX_MIN, 20, 0));
    _TEST_TRUE(wide_to_signed(L"\x10031", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"١", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"\x120" L"1", 10, 0, 0, 0));
    _TEST_TRUE(wide_to_signed(L"\xA0" L"1", 10, 0, 0, 0));
    _TEST_INT(wcstoimax(L"12", NULL, 10), 12);
}

int main(void)
{
    Test_InttypesWideSigned();
    return _Test_Status();
}
