// (Test) Status: 0
// A test file for <wchar.h> formatting into a wide string.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:S7.24.2.3] swprintf should write a format's wide characters and a null.
static void Test_WcharSwprintfText(void)
{
    wchar_t out[SIZE];
    static const wchar_t past[] = {L'a', 0xE9, 0x20AC, L'%', L'z', L'\0'};
    static const wchar_t format[] = {L'a', 0xE9, 0x20AC, L'%', L'%', L'z', L'\0'};

    put(out, L"xxxxxxxx", FILLED);
    _TEST_INT(swprintf(out, SIZE, L"abc"), 3);
    _TEST_MEM(out, L"abc\0xxxx", WIDE(8));
    _TEST_INT(swprintf(out, SIZE, L""), 0);
    _TEST_MEM(out, L"\0bc\0xxxx", WIDE(8));
    _TEST_INT((swprintf)(out, SIZE, L"100%%"), 4);
    _TEST_MEM(out, L"100%", WIDE(5));
    _TEST_INT(swprintf(out, SIZE, format), 5);
    _TEST_MEM(out, past, sizeof(past));
}

// [C99:S7.24.2.1p8] Each conversion of a number should write as fprintf's does.
static void Test_WcharSwprintfNumbers(void)
{
    wchar_t out[SIZE * 2];

    _TEST_INT(swprintf(out, SIZE, L"%d %i %u", -12, 34, 56U), 9);
    _TEST_MEM(out, L"-12 34 56", WIDE(10));
    _TEST_INT(swprintf(out, SIZE, L"%o %x %X %#x", 8U, 255U, 255U, 255U), 13);
    _TEST_MEM(out, L"10 ff FF 0xff", WIDE(14));
    _TEST_INT(swprintf(out, SIZE, L"%hhd %hd %ld %lld", 300, 70000, -5L, 6LL), 12);
    _TEST_MEM(out, L"44 4464 -5 6", WIDE(13));
    _TEST_INT(swprintf(out, SIZE, L"%jd %zu %td", (intmax_t) -7, (size_t) 8, (ptrdiff_t) 9), 6);
    _TEST_MEM(out, L"-7 8 9", WIDE(7));
    _TEST_INT(swprintf(out, SIZE, L"%f %.2f %e", 1.5, 2.345, 1234.5), 26);
    _TEST_MEM(out, L"1.500000 2.35 1.234500e+03", WIDE(27));
    _TEST_INT(swprintf(out, SIZE, L"%g %G %.3Lf", 0.00001, 1e20, 2.5L), 17);
    _TEST_MEM(out, L"1e-05 1E+20 2.500", WIDE(18));
    _TEST_INT(swprintf(out, SIZE, L"%.1a", 1.0), 8);
    _TEST_MEM(out, L"0x1.0p+0", WIDE(9));
}

// [C99:S7.24.2.1p4] A width, a precision and the flags should shape a field.
static void Test_WcharSwprintfFields(void)
{
    wchar_t out[SIZE * 2];

    _TEST_INT(swprintf(out, SIZE, L"%5d|%-5d|%05d", 42, 42, 42), 17);
    _TEST_MEM(out, L"   42|42   |00042", WIDE(18));
    _TEST_INT(swprintf(out, SIZE, L"%+d|% d|%.4d", 42, 42, 42), 12);
    _TEST_MEM(out, L"+42| 42|0042", WIDE(13));
    _TEST_INT(swprintf(out, SIZE, L"%*d|%-*d|%.*d", 4, 1, 4, 2, 3, 3), 13);
    _TEST_MEM(out, L"   1|2   |003", WIDE(14));
    _TEST_INT(swprintf(out, SIZE, L"%*d|", -4, 1), 5);
    _TEST_MEM(out, L"1   |", WIDE(6));
    _TEST_INT(swprintf(out, SIZE, L"%8.3f|%-8.1e|", 3.14159, 31.4), 18);
    _TEST_MEM(out, L"   3.142|3.1e+01 |", WIDE(19));
}

// [C99:S7.24.2.1p8] %n should store the wide characters written so far.
static void Test_WcharSwprintfCount(void)
{
    int n = NONE;
    long ln = NONE;
    wchar_t out[SIZE];
    signed char cn = NONE;
    static const wchar_t format[] = {L'a', 0x20AC, L'%', L'n', L'b', L'%', L'l', L'n', L'%', L'h', L'h', L'n', L'\0'};

    _TEST_INT(swprintf(out, SIZE, format, &n, &ln, &cn), 3);
    _TEST_INT(n, 2);
    _TEST_INT(ln, 3);
    _TEST_INT(cn, 3);
}

int main(void)
{
    Test_WcharSwprintfText();
    Test_WcharSwprintfNumbers();
    Test_WcharSwprintfFields();
    Test_WcharSwprintfCount();
    return _Test_Status();
}
