// (Test) Status: 0
// A test file for <string.h> concatenation functions.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// A concatenation should write past a string's end, a null after.
// [C99:7.21.3]: strcat and strncat
static void Test_StringConcatenation(void)
{
    char buf[SIZE];

    put(buf, "ab\0xxxxx", FILLED);
    _TEST_TRUE(strcat(buf, "cd") == buf);
    _TEST_MEM(buf, "abcd\0xxx", FILLED);
    _TEST_TRUE(strcat(buf, "") == buf);
    _TEST_MEM(buf, "abcd\0xxx", FILLED);

    put(buf, "ab\0xxxxx", FILLED);
    _TEST_TRUE(strncat(buf, "cdef", 2) == buf);
    _TEST_MEM(buf, "abcd\0xxx", FILLED);
    _TEST_TRUE(strncat(buf, "e", 5) == buf);
    _TEST_MEM(buf, "abcde\0xx", FILLED);
    _TEST_TRUE(strncat(buf, "z", 0) == buf);
    _TEST_MEM(buf, "abcde\0xx", FILLED);
}

int main(void)
{
    Test_StringConcatenation();
    return _Test_Status();
}
