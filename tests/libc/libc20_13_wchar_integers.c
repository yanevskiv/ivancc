// (Test) Status: 0
// A test file for <wchar.h> wcstol and its kin.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99/S7.24.4.1.2p5] wcstol should read a wide string in a base, and stop.
static bool converts(const wchar_t *str, int base, long value, long end)
{
    int lost = 0;
    wchar_t *stop = NULL;
    long got = wcstol(str, &stop, base);

    lost += ! _TEST_INT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99/S7.24.4.1.2p8] wcstol and its kin should read any base, a limit past range.
static void Test_WcharIntegers(void)
{
    wchar_t *stop = NULL;
    const wchar_t *most = L"99999999999999999999";

    _TEST_TRUE(converts(L"  -123abc", 10, -123, 6));
    _TEST_TRUE(converts(L"0x1F", 0, 31, 4));
    _TEST_TRUE(converts(L"017", 0, 15, 3));
    _TEST_TRUE(converts(L"z", 36, 35, 1));
    _TEST_TRUE(converts(L"101", 2, 5, 3));
    _TEST_TRUE(converts(L"-0x10", 16, -16, 5));
    _TEST_TRUE(converts(L"0x", 16, 0, 1));
    _TEST_TRUE(converts(L"09", 0, 0, 1));
    _TEST_TRUE(converts(L"  +", 10, 0, 0));
    _TEST_TRUE(converts(L"", 0, 0, 0));
    _TEST_TRUE(converts(L"\t\n\v\f\r 7", 10, 7, 7));
    _TEST_TRUE(converts(L"12\x20AC", 10, 12, 2));
    _TEST_TRUE(converts(L"\x10031", 10, 0, 0));

    errno = 0;
    _TEST_INT(wcstol(most, &stop, 10), LONG_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(stop - most, 20);
    errno = 0;
    _TEST_INT(wcstoll(L"-99999999999999999999", NULL, 10), LLONG_MIN);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_INT(wcstol(L"-99999999999999999999", NULL, 0), LONG_MIN);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_INT(wcstol(L"9223372036854775807", NULL, 10), LONG_MAX);
    _TEST_INT(wcstoll(L"-9223372036854775808", NULL, 10), LLONG_MIN);
    _TEST_INT(errno, 0);
    _TEST_INT(wcstoll(L"12", NULL, 0), 12);
    _TEST_UINT(wcstoul(L"-1", NULL, 10), ULONG_MAX);
    _TEST_UINT(wcstoull(L"-1", NULL, 10), ULLONG_MAX);
    _TEST_UINT(wcstoul(L"0xffff", NULL, 16), 0xffff);
    _TEST_UINT(wcstoull(L"18446744073709551615", &stop, 10), ULLONG_MAX);
    _TEST_INT(errno, 0);
    errno = 0;
    _TEST_UINT(wcstoull(L"999999999999999999999999", NULL, 10), ULLONG_MAX);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_UINT(wcstoul(L"18446744073709551616", NULL, 10), ULONG_MAX);
    _TEST_INT(errno, ERANGE);
}

// [C99/S7.1.4p1] A call in parentheses should reach each function.
static void Test_WcharIntegerCalls(void)
{
    wchar_t *stop = NULL;
    const wchar_t *text = L"42x";

    _TEST_INT((wcstol)(text, &stop, 10), 42);
    _TEST_INT(stop - text, 2);
    _TEST_INT((wcstoll)(text, &stop, 10), 42);
    _TEST_INT(stop - text, 2);
    _TEST_UINT((wcstoul)(text, &stop, 10), 42);
    _TEST_INT(stop - text, 2);
    _TEST_UINT((wcstoull)(text, &stop, 10), 42);
    _TEST_INT(stop - text, 2);
}

int main(void)
{
    Test_WcharIntegers();
    Test_WcharIntegerCalls();
    return _Test_Status();
}
