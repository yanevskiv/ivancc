// (Test) Status: 0
// A test file for <wchar.h> wide string comparison functions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:7.24.4.4p1] A comparison should take the sign of its first differing pair.
static void Test_WcharComparison(void)
{
    _TEST_INT(wcscmp(L"abc", L"abc"), 0);
    _TEST_INT(sign(wcscmp(L"abc", L"abd")), -1);
    _TEST_INT(sign(wcscmp(L"abd", L"abc")), 1);
    _TEST_INT(sign(wcscmp(L"ab", L"abc")), -1);
    _TEST_INT(sign(wcscmp(L"abc", L"ab")), 1);
    _TEST_INT(wcscmp(L"", L""), 0);
    _TEST_INT(sign(wcscmp(L"\x20AC", L"a")), 1);
    _TEST_INT(sign(wcscmp(L"a", L"\x10FFFF")), -1);

    _TEST_INT(wcsncmp(L"abcx", L"abcy", 3), 0);
    _TEST_INT(sign(wcsncmp(L"abcx", L"abcy", 4)), -1);
    _TEST_INT(sign(wcsncmp(L"ab", L"abc", 5)), -1);
    _TEST_INT(wcsncmp(L"ab\0x", L"ab\0y", 4), 0);
    _TEST_INT(wcsncmp(L"a", L"b", 0), 0);
    _TEST_INT(sign(wcsncmp(L"\x20AC", L"a", 1)), 1);

    _TEST_INT(sign(wmemcmp(L"abc", L"abd", 3)), -1);
    _TEST_INT(sign(wmemcmp(L"abd", L"abc", 3)), 1);
    _TEST_INT(wmemcmp(L"abc", L"abd", 2), 0);
    _TEST_INT(wmemcmp(L"ab\0c", L"ab\0c", 4), 0);
    _TEST_INT(sign(wmemcmp(L"ab\0c", L"ab\0d", 4)), -1);
    _TEST_INT(wmemcmp(L"a", L"b", 0), 0);
    _TEST_INT(sign(wmemcmp(L"\x20AC", L"a", 1)), 1);
}

// [C99:7.24.4.4p1] A comparison should order the least wchar_t below the greatest.
static void Test_WcharComparisonLimits(void)
{
    static const wchar_t least[] = {L'a', WCHAR_MIN, L'\0'};
    static const wchar_t most[] = {L'a', WCHAR_MAX, L'\0'};

    _TEST_INT(sign(wcscmp(least, most)), -1);
    _TEST_INT(sign(wcscmp(most, least)), 1);
    _TEST_INT(sign(wcsncmp(least, most, 2)), -1);
    _TEST_INT(sign(wcsncmp(most, least, 2)), 1);
    _TEST_INT(sign(wmemcmp(least, most, 2)), -1);
    _TEST_INT(sign(wmemcmp(most, least, 2)), 1);
    _TEST_INT(wmemcmp(least, most, 1), 0);
}

// [C99:7.24.4.4.2] wcscoll should order, and wcsxfrm give what wcscmp orders so.
static void Test_WcharCollation(void)
{
    size_t len;
    wchar_t buf[SIZE];
    wchar_t other[SIZE];

    _TEST_INT(sign(wcscoll(L"abc", L"abd")), -1);
    _TEST_INT(sign(wcscoll(L"b", L"a")), 1);
    _TEST_INT(wcscoll(L"abc", L"abc"), 0);
    _TEST_INT(sign(wcscoll(L"ab", L"abc")), -1);

    len = wcsxfrm(buf, L"abd", SIZE);
    _TEST_UINT(wcsxfrm(NULL, L"abd", 0), len);
    if (_TEST_TRUE(len < SIZE)) {
        _TEST_UINT(wcslen(buf), len);
        len = wcsxfrm(other, L"abc", SIZE);
        if (_TEST_TRUE(len < SIZE)) {
            _TEST_INT(sign(wcscmp(other, buf)), sign(wcscoll(L"abc", L"abd")));
        }
    }
}

int main(void)
{
    Test_WcharComparison();
    Test_WcharComparisonLimits();
    Test_WcharCollation();
    return _Test_Status();
}
