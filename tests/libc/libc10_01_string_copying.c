// (Test) Status: 0
// A test file for <string.h> copying functions.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// [C99 S7.21.2] A copy should write the bytes asked for alone, and return where.
static void Test_StringCopying(void)
{
    char buf[SIZE];

    put(buf, "xxxxxxxx", FILLED);
    _TEST_TRUE(memcpy(buf, "ab\0cd", 5) == buf);
    _TEST_MEM(buf, "ab\0cdxxx", FILLED);
    _TEST_TRUE(memcpy(buf, "z", 0) == buf);
    _TEST_MEM(buf, "ab\0cdxxx", FILLED);

    put(buf, "abcdefgh", FILLED);
    _TEST_TRUE(memmove(buf + 2, buf, 5) == buf + 2);
    _TEST_MEM(buf, "ababcdeh", FILLED);
    put(buf, "abcdefgh", FILLED);
    _TEST_TRUE(memmove(buf, buf + 2, 5) == buf);
    _TEST_MEM(buf, "cdefgfgh", FILLED);
    _TEST_TRUE(memmove(buf, buf, FILLED) == buf);
    _TEST_MEM(buf, "cdefgfgh", FILLED);

    put(buf, "xxxxxxxx", FILLED);
    _TEST_TRUE(strcpy(buf, "abc") == buf);
    _TEST_MEM(buf, "abc\0xxxx", FILLED);
    _TEST_TRUE(strcpy(buf, "") == buf);
    _TEST_MEM(buf, "\0bc\0xxxx", FILLED);

    put(buf, "xxxxxxxx", FILLED);
    _TEST_TRUE(strncpy(buf, "ab", 5) == buf);
    _TEST_MEM(buf, "ab\0\0\0xxx", FILLED);
    _TEST_TRUE(strncpy(buf, "abcdefg", 3) == buf);
    _TEST_MEM(buf, "abc\0\0xxx", FILLED);
    _TEST_TRUE(strncpy(buf, "z", 0) == buf);
    _TEST_MEM(buf, "abc\0\0xxx", FILLED);
}

int main(void)
{
    Test_StringCopying();
    return _Test_Status();
}
