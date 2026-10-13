// (Test) Status: 0
// A test file for <wchar.h> wcstof and wcstold.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99 S7.24.4.1.1p4] wcstof should read a wide string as a value, and stop.
static bool reads_float(const wchar_t *str, float value, long end)
{
    int lost = 0;
    wchar_t *stop = NULL;
    float got = wcstof(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99 S7.24.4.1.1p4] wcstold should read a wide string as a value, and stop.
static bool reads_long(const wchar_t *str, long double value, long end)
{
    int lost = 0;
    wchar_t *stop = NULL;
    long double got = wcstold(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// [C99 S7.24.4.1.1p10] wcstof should read to the nearest float, and range.
static void Test_WcharWcstof(void)
{
    _TEST_TRUE(reads_float(L"1.5", 1.5F, 3));
    _TEST_TRUE(reads_float(L"3.4028235e38", FLT_MAX, 12));
    _TEST_TRUE(reads_float(L"0x1p-126", FLT_MIN, 8));
    _TEST_TRUE(reads_float(L"0x1.000001p0", 1.0F, 12));
    _TEST_TRUE(reads_float(L"0x1.000003p0", 0x1.000004p0F, 12));
    _TEST_TRUE(reads_float(L"0.1", 0.1F, 3));
    _TEST_TRUE(reads_float(L" -2.5z", -2.5F, 5));
    errno = 0;
    _TEST_TRUE(wcstof(L"3.5e38", NULL) > FLT_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_TRUE(wcstof(L"1e-50", NULL) <= FLT_MIN);
    _TEST_TRUE((wcstof)(L"2.5x", NULL) == 2.5F);
}

// [C99 S7.24.4.1.1p10] wcstold should read to the nearest long double, and range.
static void Test_WcharWcstold(void)
{
    _TEST_TRUE(reads_long(L"1.5", 1.5L, 3));
    _TEST_TRUE(reads_long(L"0.1", 0.1L, 3));
    _TEST_TRUE(reads_long(L"-0x1p-16382", -LDBL_MIN, 11));
    _TEST_TRUE(reads_long(L"0x1.fffffffffffffffep16383", LDBL_MAX, 26));
    _TEST_TRUE(reads_long(L"3.36210314311209350626e-4932", LDBL_MIN, 28));
    errno = 0;
    _TEST_TRUE(wcstold(L"1e5000", NULL) > LDBL_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_TRUE((wcstold)(L"2.5x", NULL) == 2.5L);
}

int main(void)
{
    Test_WcharWcstof();
    Test_WcharWcstold();
    return _Test_Status();
}
