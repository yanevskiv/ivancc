// (Test) Status: 0
// A test file for <wchar.h> wcsftime.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8      "C.UTF-8"
#define TEXT_SIZE 256
#define LAST_1999 946684798L

// [C99 S7.24.5.1p2] wcsftime should format a time in UTC as the wide text given.
static bool formats(const wchar_t *format, const wchar_t *want)
{
    int lost = 0;
    time_t when = LAST_1999;
    struct tm tm = *gmtime(&when);
    wchar_t buf[TEXT_SIZE];
    size_t len = wcsftime(buf, TEXT_SIZE, format, &tm);

    if (len == 0) {
        buf[0] = L'\0';
    }
    lost += ! _TEST_UINT(len, wcslen(want));
    lost += ! _TEST_MEM(buf, want, WIDE(wcslen(want) + 1));
    return lost == 0;
}

// [C99 S7.24.5.1p2] wcsftime should write each conversion as strftime does.
static void Test_WcharWcsftimeConversions(void)
{
    _TEST_TRUE(formats(L"%a|%A|%b|%B|%c|%C|%d|%D|%e|%F|%g|%G|%h|%H|%I|%j|%m", L"Fri|Friday|Dec|December|Fri Dec 31 23:59:58 1999|19|31|12/31/99|31|1999-12-31|99|1999|Dec|23|11|365|12"));
    _TEST_TRUE(formats(L"%M|%n|%p|%r|%R|%S|%t|%T|%u|%U|%V|%w|%W|%x|%X|%y|%Y|%%", L"59|\n|PM|11:59:58 PM|23:59|58|\t|23:59:58|5|52|52|5|52|12/31/99|23:59:58|99|1999|%"));
    _TEST_TRUE(formats(L"%Ec|%EC|%Ex|%EX|%Ey|%EY", L"Fri Dec 31 23:59:58 1999|19|12/31/99|23:59:58|99|1999"));
    _TEST_TRUE(formats(L"%Od|%Oe|%OH|%OI|%Om|%OM|%OS|%Ou|%OU|%OV|%Ow|%OW|%Oy", L"31|31|23|11|12|59|58|5|52|52|5|52|99"));
    _TEST_TRUE(formats(L"", L""));
    _TEST_TRUE(formats(L"plain text", L"plain text"));
    _TEST_TRUE(formats(L"%Y%m%d", L"19991231"));
    _TEST_TRUE(formats(L"a%%b%nc", L"a%b\nc"));
}

// [C99 S7.24.5.1p3] wcsftime should count wide characters, nothing if too many.
static void Test_WcharWcsftimeSizes(void)
{
    time_t when = LAST_1999;
    struct tm tm = *gmtime(&when);
    wchar_t buf[TEXT_SIZE];

    put(buf, L"xxxxxxxx", FILLED);
    _TEST_UINT(wcsftime(buf, 5, L"%Y", &tm), 4);
    _TEST_MEM(buf, L"1999\0xxx", WIDE(8));
    _TEST_UINT(wcsftime(buf, 4, L"%Y", &tm), 0);
    _TEST_UINT(wcsftime(buf, 0, L"%Y", &tm), 0);
    _TEST_UINT(wcsftime(buf, 3, L"abc", &tm), 0);
    _TEST_UINT(wcsftime(buf, 4, L"abc", &tm), 3);
    _TEST_MEM(buf, L"abc", WIDE(4));
    _TEST_UINT(wcsftime(buf, 1, L"", &tm), 0);
    _TEST_INT(buf[0], L'\0');
    _TEST_UINT((wcsftime)(buf, TEXT_SIZE, L"%H:%M", &tm), 5);
    _TEST_MEM(buf, L"23:59", WIDE(6));
}

// [C99 S7.24.5.1p2] wcsftime should copy a wide character of any value as it is.
static void Test_WcharWcsftimeWide(void)
{
    if (! _TEST_TRUE(setlocale(LC_ALL, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(formats(L"\x20AC%Y\xE9%m\x1F600", L"\x20AC" L"1999\xE9" L"12\x1F600"));
    _TEST_TRUE(formats(L"%d\x4E2D", L"31\x4E2D"));
}

int main(void)
{
    Test_WcharWcsftimeConversions();
    Test_WcharWcsftimeSizes();
    Test_WcharWcsftimeWide();
    return _Test_Status();
}
