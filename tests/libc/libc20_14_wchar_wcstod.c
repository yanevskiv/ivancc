// (Test) Status: 0
// A test file for <wchar.h> wcstod.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

static volatile double nought = 0.0;

// [C99:7.24.4.1.1p4] wcstod should read a wide string as a value, and stop.
static bool reads(const wchar_t *str, double value, long end)
{
    int lost = 0;
    wchar_t *stop = NULL;
    double got = wcstod(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99:7.24.4.1.1p3] wcstod should read each decimal and hexadecimal form alone.
static void Test_WcharWcstodForms(void)
{
    _TEST_TRUE(reads(L"1.5", 1.5, 3));
    _TEST_TRUE(reads(L"  -2.25e1", -22.5, 9));
    _TEST_TRUE(reads(L"+.5", 0.5, 3));
    _TEST_TRUE(reads(L"5.", 5.0, 2));
    _TEST_TRUE(reads(L"12abc", 12.0, 2));
    _TEST_TRUE(reads(L"\t\n\v\f\r 7", 7.0, 7));
    _TEST_TRUE(reads(L"1.5.5", 1.5, 3));
    _TEST_TRUE(reads(L"0x1.8.8", 1.5, 5));
    _TEST_TRUE(reads(L"1e", 1.0, 1));
    _TEST_TRUE(reads(L"1e+", 1.0, 1));
    _TEST_TRUE(reads(L"1e+x", 1.0, 1));
    _TEST_TRUE(reads(L"1.5E+00002", 150.0, 10));
    _TEST_TRUE(reads(L"0x", 0.0, 1));
    _TEST_TRUE(reads(L"0x1.8p1", 3.0, 7));
    _TEST_TRUE(reads(L"0X1P-2", 0.25, 6));
    _TEST_TRUE(reads(L"0x1p", 1.0, 3));
    _TEST_TRUE(reads(L"0x.8", 0.5, 4));
    _TEST_TRUE(reads(L"-0xAbp0", -171.0, 7));
    _TEST_TRUE(reads(L"1.5\x20AC", 1.5, 3));

    _TEST_TRUE(reads(L"", 0.0, 0));
    _TEST_TRUE(reads(L".", 0.0, 0));
    _TEST_TRUE(reads(L"-", 0.0, 0));
    _TEST_TRUE(reads(L"e5", 0.0, 0));
    _TEST_TRUE(reads(L".e1", 0.0, 0));
    _TEST_TRUE(reads(L"x", 0.0, 0));
    _TEST_TRUE(reads(L"\x10031", 0.0, 0));
}

// [C99:7.24.4.1.1p3] wcstod should read an infinity and a NaN, in any case.
static void Test_WcharWcstodSpecials(void)
{
    double invalid = nought / nought;
    double infinite = 1 / nought;

    _TEST_TRUE(reads(L"inf", infinite, 3));
    _TEST_TRUE(reads(L"INFINITY", infinite, 8));
    _TEST_TRUE(reads(L"-Inf", -infinite, 4));
    _TEST_TRUE(reads(L"infinit", infinite, 3));
    _TEST_TRUE(reads(L"nan", invalid, 3));
    _TEST_TRUE(reads(L"-NaN", invalid, 4));
    _TEST_TRUE(reads(L"nan()", invalid, 5));
    _TEST_TRUE(reads(L"nan(123)", invalid, 8));
    _TEST_TRUE(reads(L"NAN(abc_9)", invalid, 10));
    _TEST_TRUE(reads(L"nan(", invalid, 3));
    _TEST_TRUE(reads(L"nan(-1)", invalid, 3));
}

// [C99:7.24.4.1.1p10] wcstod should round to nearest, and range with ERANGE.
static void Test_WcharWcstodRange(void)
{
    wchar_t *stop = NULL;
    const wchar_t *huge = L"-1e400";

    _TEST_TRUE(reads(L"1.7976931348623157e308", DBL_MAX, 22));
    _TEST_TRUE(reads(L"2.2250738585072014e-308", DBL_MIN, 23));
    _TEST_TRUE(reads(L"0x1.00000000000008p0", 1.0, 20));
    _TEST_TRUE(reads(L"0x1.00000000000018p0", 0x1.0000000000002p0, 20));
    _TEST_TRUE(reads(L"0.1", 0.1, 3));
    errno = 0;
    _TEST_TRUE(wcstod(L"1e400", NULL) > DBL_MAX);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_TRUE(wcstod(huge, &stop) < -DBL_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(stop - huge, 6);
    _TEST_TRUE(wcstod(L"1e-400", NULL) <= DBL_MIN);
    _TEST_TRUE((wcstod)(L"2.5x", NULL) == 2.5);
}

int main(void)
{
    Test_WcharWcstodForms();
    Test_WcharWcstodSpecials();
    Test_WcharWcstodRange();
    return _Test_Status();
}
