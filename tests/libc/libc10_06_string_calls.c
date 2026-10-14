// (Test) Status: 0
// A test file for <string.h> calls by pointer and past a macro.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// [C99:7.1.4p1] A pointer and a call in parentheses should reach each function.
static void Test_StringCalls(void)
{
    char buf[SIZE];
    size_t (*length)(const char *) = strlen;
    void *(*copy)(void *restrict, const void *restrict, size_t) = memcpy;

    _TEST_TRUE(copy(buf, "zy", 3) == buf);
    _TEST_STR(buf, "zy");
    _TEST_UINT(length("abcd"), 4);

    _TEST_TRUE((memcpy)(buf, "ab", 3) == buf);
    _TEST_STR(buf, "ab");
    _TEST_TRUE((memmove)(buf + 1, buf, 2) == buf + 1);
    _TEST_MEM(buf, "aab", 3);
    _TEST_STR((strcpy)(buf, "abc"), "abc");
    _TEST_STR((strncpy)(buf, "xy", 2), "xyc");
    _TEST_STR((strcat)(buf, "d"), "xycd");
    _TEST_STR((strncat)(buf, "efg", 1), "xycde");
    _TEST_INT((memcmp)("a", "a", 1), 0);
    _TEST_INT((strcmp)("a", "a"), 0);
    _TEST_INT((strcoll)("a", "a"), 0);
    _TEST_INT((strncmp)("ab", "ac", 1), 0);
    _TEST_UINT((strxfrm)(NULL, "", 0), 0);
    _TEST_INT(offset(buf, (memchr)(buf, 'c', 5)), 2);
    _TEST_INT(offset(buf, (strchr)(buf, 'd')), 3);
    _TEST_UINT((strcspn)(buf, "e"), 4);
    _TEST_INT(offset(buf, (strpbrk)(buf, "ey")), 1);
    _TEST_INT(offset(buf, (strrchr)(buf, 'x')), 0);
    _TEST_UINT((strspn)(buf, "yx"), 2);
    _TEST_INT(offset(buf, (strstr)(buf, "de")), 3);
    _TEST_INT(offset(buf, (strtok)(buf, "c")), 0);
    _TEST_STR(buf, "xy");
    _TEST_TRUE((memset)(buf, 'q', 1) == buf);
    _TEST_STR(buf, "qy");
    _TEST_STR((strerror)(0), "Success");
    _TEST_UINT((strlen)("abcd"), 4);
}

int main(void)
{
    Test_StringCalls();
    return _Test_Status();
}
