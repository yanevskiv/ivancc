// (Test) Status: 0
// A test file for <wchar.h> wide string concatenation functions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// [C99:7.24.4.3] A concatenation should write past a string's end, a null after.
static void Test_WcharConcatenation(void)
{
    wchar_t buf[SIZE];

    put(buf, L"ab\0xxxxx", FILLED);
    _TEST_TRUE(wcscat(buf, L"cd") == buf);
    _TEST_MEM(buf, L"abcd\0xxx", WIDE(FILLED));
    _TEST_TRUE(wcscat(buf, L"") == buf);
    _TEST_MEM(buf, L"abcd\0xxx", WIDE(FILLED));

    put(buf, L"ab\0xxxxx", FILLED);
    _TEST_TRUE(wcsncat(buf, L"cdef", 2) == buf);
    _TEST_MEM(buf, L"abcd\0xxx", WIDE(FILLED));
    _TEST_TRUE(wcsncat(buf, L"e", 5) == buf);
    _TEST_MEM(buf, L"abcde\0xx", WIDE(FILLED));
    _TEST_TRUE(wcsncat(buf, L"z", 0) == buf);
    _TEST_MEM(buf, L"abcde\0xx", WIDE(FILLED));
}

int main(void)
{
    Test_WcharConcatenation();
    return _Test_Status();
}
