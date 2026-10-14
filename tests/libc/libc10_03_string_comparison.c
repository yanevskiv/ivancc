// (Test) Status: 0
// A test file for <string.h> comparison functions.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// Return -1, 0 or 1 as a comparison's result is negative, zero or positive.
static int sign(int value)
{
    return (value > 0) - (value < 0);
}

// A comparison should take the sign of its first differing bytes.
// [C99:7.21.4p1]: a comparison's sign, by unsigned char
static void Test_StringComparison(void)
{
    size_t len;
    char buf[SIZE];
    char other[SIZE];

    _TEST_INT(sign(memcmp("abc", "abd", 3)), -1);
    _TEST_INT(sign(memcmp("abd", "abc", 3)), 1);
    _TEST_INT(memcmp("abc", "abd", 2), 0);
    _TEST_INT(memcmp("ab\0c", "ab\0c", 4), 0);
    _TEST_INT(sign(memcmp("ab\0c", "ab\0d", 4)), -1);
    _TEST_INT(memcmp("a", "b", 0), 0);
    _TEST_INT(sign(memcmp("\x80", "\x01", 1)), 1);

    _TEST_INT(strcmp("abc", "abc"), 0);
    _TEST_INT(sign(strcmp("abc", "abd")), -1);
    _TEST_INT(sign(strcmp("abd", "abc")), 1);
    _TEST_INT(sign(strcmp("ab", "abc")), -1);
    _TEST_INT(sign(strcmp("abc", "ab")), 1);
    _TEST_INT(strcmp("", ""), 0);
    _TEST_INT(sign(strcmp("\x80", "a")), 1);

    _TEST_INT(sign(strcoll("abc", "abd")), -1);
    _TEST_INT(sign(strcoll("b", "a")), 1);
    _TEST_INT(strcoll("abc", "abc"), 0);
    _TEST_INT(sign(strcoll("ab", "abc")), -1);
    _TEST_INT(sign(strcoll("\x80", "a")), 1);

    _TEST_INT(strncmp("abcx", "abcy", 3), 0);
    _TEST_INT(sign(strncmp("abcx", "abcy", 4)), -1);
    _TEST_INT(sign(strncmp("ab", "abc", 5)), -1);
    _TEST_INT(strncmp("ab\0x", "ab\0y", 4), 0);
    _TEST_INT(strncmp("a", "b", 0), 0);
    _TEST_INT(sign(strncmp("\x80", "a", 1)), 1);

    len = strxfrm(buf, "abd", sizeof(buf));
    _TEST_UINT(strxfrm(NULL, "abd", 0), len);
    if (_TEST_TRUE(len < sizeof(buf))) {
        _TEST_UINT(strlen(buf), len);
        len = strxfrm(other, "abc", sizeof(other));
        if (_TEST_TRUE(len < sizeof(other))) {
            _TEST_INT(sign(strcmp(other, buf)), sign(strcoll("abc", "abd")));
        }
    }
}

int main(void)
{
    Test_StringComparison();
    return _Test_Status();
}
