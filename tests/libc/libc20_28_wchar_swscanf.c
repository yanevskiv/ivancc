// (Test) Status: 0
// A test file for <wchar.h> reading a wide string by a format.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:7.24.2.2p12] Each conversion of a number should read as fscanf's does.
static void Test_WcharSwscanfNumbers(void)
{
    int i = 0;
    int j = 0;
    long l = 0;
    float f = 0;
    short h = 0;
    double d = 0;
    size_t z = 0;
    void *p = NULL;
    long long ll = 0;
    intmax_t max = 0;
    static int object;
    unsigned int o = 0;
    unsigned int u = 0;
    unsigned int x = 0;
    long double ld = 0;
    signed char hh = 0;
    wchar_t address[SIZE];

    _TEST_INT(swscanf(L"12 -34", L"%d%d", &i, &j), 2);
    _TEST_INT(i, 12);
    _TEST_INT(j, -34);
    _TEST_INT(swscanf(L"0x1f 017 9", L"%i%i%i", &i, &j, &u), 3);
    _TEST_INT(i, 31);
    _TEST_INT(j, 15);
    _TEST_UINT(u, 9);
    _TEST_INT(swscanf(L"17 42 fF", L"%o%u%X", &o, &u, &x), 3);
    _TEST_UINT(o, 15);
    _TEST_UINT(u, 42);
    _TEST_UINT(x, 255);
    _TEST_INT(swscanf(L"-1 -2 -3 -4 -5 6", L"%hhd%hd%ld%lld%jd%zu", &hh, &h, &l, &ll, &max, &z), 6);
    _TEST_INT(hh, -1);
    _TEST_INT(h, -2);
    _TEST_INT(l, -3);
    _TEST_INT(ll, -4);
    _TEST_INT(max, -5);
    _TEST_UINT(z, 6);
    _TEST_INT(swscanf(L"1.5 -2.5e1 0x1p3", L"%f%lf%Lf", &f, &d, &ld), 3);
    _TEST_FLOAT(f, 1.5);
    _TEST_FLOAT(d, -25.0);
    _TEST_FLOAT(ld, 8.0);
    _TEST_INT(swscanf(L"1E2 .5 3.", L"%E%lG%La", &f, &d, &ld), 3);
    _TEST_FLOAT(f, 100.0);
    _TEST_FLOAT(d, 0.5);
    _TEST_FLOAT(ld, 3.0);
    _TEST_INT(swscanf(L"12345", L"%2d%3d", &i, &j), 2);
    _TEST_INT(i, 12);
    _TEST_INT(j, 345);
    _TEST_TRUE(swprintf(address, SIZE, L"%p", (void *) &object) > 0);
    _TEST_INT(swscanf(address, L"%p", &p), 1);
    _TEST_TRUE(p == (void *) &object);
}

// [C99:7.24.2.2p5] A format's white space and wide characters should match input.
static void Test_WcharSwscanfText(void)
{
    int i = 0;
    int j = 0;
    int n = NONE;
    long ln = NONE;
    static const wchar_t input[] = {0x20AC, L'7', 0xE9, L'8', L'\0'};
    static const wchar_t format[] = {0x20AC, L'%', L'd', 0xE9, L'%', L'd', L'\0'};
    static const wchar_t other[] = {0x20AC, L'%', L'd', 0xE8, L'%', L'd', L'\0'};

    _TEST_INT(swscanf(L"a=1,b=2", L"a=%d,b=%d", &i, &j), 2);
    _TEST_INT(i, 1);
    _TEST_INT(j, 2);
    _TEST_INT(swscanf(L"3 \t\n 4", L"%d %d", &i, &j), 2);
    _TEST_INT(j, 4);
    _TEST_INT(swscanf(L"5:6", L"%d : %d", &i, &j), 2);
    _TEST_INT(j, 6);
    _TEST_INT((swscanf)(L"50% 7", L"%d%% %d%n", &i, &j, &n), 2);
    _TEST_INT(i, 50);
    _TEST_INT(j, 7);
    _TEST_INT(n, 5);
    _TEST_INT(swscanf(input, format, &i, &j), 2);
    _TEST_INT(i, 7);
    _TEST_INT(j, 8);
    j = NONE;
    _TEST_INT(swscanf(input, other, &i, &j), 1);
    _TEST_INT(j, NONE);
    _TEST_INT(swscanf(L"12 34", L"%*d%n%d%ln", &n, &i, &ln), 1);
    _TEST_INT(n, 2);
    _TEST_INT(i, 34);
    _TEST_INT(ln, 5);
}

// [C99:7.24.2.4p3] swscanf should return EOF for no input, else the items stored.
static void Test_WcharSwscanfFailures(void)
{
    int i = NONE;
    int j = NONE;

    _TEST_INT(swscanf(L"", L"%d", &i), EOF);
    _TEST_INT(swscanf(L"   ", L"%d", &i), EOF);
    _TEST_INT(swscanf(L"x", L"%d", &i), 0);
    _TEST_INT(swscanf(L"1 x", L"%d%d", &i, &j), 1);
    _TEST_INT(i, 1);
    _TEST_INT(j, NONE);
    _TEST_INT(swscanf(L"2", L"%d%d", &i, &j), 1);
    _TEST_INT(i, 2);
    _TEST_INT(swscanf(L"3b", L"%da%d", &i, &j), 1);
    _TEST_INT(swscanf(L"", L"x%d", &i), EOF);
    _TEST_INT(swscanf(L"-", L"%d", &i), 0);
    _TEST_INT(swscanf(L"anything", L""), 0);
}

int main(void)
{
    Test_WcharSwscanfNumbers();
    Test_WcharSwscanfText();
    Test_WcharSwscanfFailures();
    return _Test_Status();
}
