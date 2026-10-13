// (Test) Status: 0
// A test file for <wchar.h> formatting into a wide string of a size.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:S7.24.2.3p2] swprintf should write no more than n wide characters.
static void Test_WcharSwprintfSize(void)
{
    wchar_t out[SIZE];

    put(out, L"xxxxxxxx", FILLED);
    _TEST_INT(swprintf(out, 4, L"abc"), 3);
    _TEST_MEM(out, L"abc\0xxxx", WIDE(8));
    put(out, L"xxxxxxxx", FILLED);
    _TEST_INT(swprintf(out, 6, L"%d|%d", 12, 34), 5);
    _TEST_MEM(out, L"12|34\0xx", WIDE(8));
    put(out, L"xxxxxxxx", FILLED);
    _TEST_INT(swprintf(out, 1, L""), 0);
    _TEST_MEM(out, L"\0xxxxxxx", WIDE(8));
}

// [C99:S7.24.2.3p3] swprintf should fail where n or more were to be written.
static void Test_WcharSwprintfOverflow(void)
{
    wchar_t out[SIZE];

    put(out, L"xxxxxxxx", FILLED);
    _TEST_TRUE(swprintf(out, 3, L"abc") < 0);
    _TEST_MEM(out + 3, L"xxxxx", WIDE(5));
    _TEST_TRUE(swprintf(out, 2, L"%d", 123) < 0);
    _TEST_MEM(out + 3, L"xxxxx", WIDE(5));
    _TEST_TRUE(swprintf(out, 4, L"%8d", 1) < 0);
    _TEST_MEM(out + 4, L"xxxx", WIDE(4));
    _TEST_TRUE(swprintf(out, 0, L"") < 0);
    _TEST_TRUE(swprintf(out, 0, L"abc") < 0);
    _TEST_MEM(out + 4, L"xxxx", WIDE(4));
}

int main(void)
{
    Test_WcharSwprintfSize();
    Test_WcharSwprintfOverflow();
    return _Test_Status();
}
