// (Test) Status: 0
// <string.h> declares the copying functions memcpy, memmove, strcpy and strncpy (S7.21.2), the concatenation functions strcat and strncat (S7.21.3),
// the comparison functions memcmp, strcmp, strcoll, strncmp and strxfrm (S7.21.4), the search functions memchr, strchr, strcspn, strpbrk, strrchr, strspn, strstr and strtok (S7.21.5),
// and memset, strerror and strlen (S7.21.6), each writing the bytes it is asked for and no other, comparing bytes as unsigned char (S7.21.4p1) and taking a character as an int it converts,
// each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#define NONE   (-1)
#define SIZE   32
#define FILLED 9

// Fill a buffer's first bytes by no function under test.
static void put(char *to, const char *from, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        to[i] = from[i];
    }
}

// Return -1, 0 or 1 as a comparison's result is negative, zero or positive.
static int sign(int value)
{
    return (value > 0) - (value < 0);
}

// Return how far into a string a search's result points, or NONE for null.
static long offset(const char *str, const void *found)
{
    return found == NULL ? NONE : (const char *) found - str;
}

// A copy writes the bytes asked for and no other, and returns where it wrote.
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

// A concatenation writes past a string's end, with a null character after.
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

// A comparison's sign is that of the first bytes to differ, as unsigned char.
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

// A search points at what it finds, or finds nothing, and counts what it spans.
static void Test_StringSearch(void)
{
    char buf[SIZE];
    const char *word = "abcabc";
    const char *bytes = "ab\0cab";
    static char text[] = "?a???b,,,#c";

    _TEST_INT(offset(bytes, memchr(bytes, 'b', 6)), 1);
    _TEST_INT(offset(bytes, memchr(bytes, 'c', 6)), 3);
    _TEST_INT(offset(bytes, memchr(bytes, '\0', 6)), 2);
    _TEST_INT(offset(bytes, memchr(bytes, 'c', 3)), NONE);
    _TEST_INT(offset(bytes, memchr(bytes, 'z', 6)), NONE);
    _TEST_INT(offset(bytes, memchr(bytes, 'a', 0)), NONE);
    _TEST_INT(offset(bytes, memchr(bytes, 0x100 + 'b', 6)), 1);
    _TEST_TRUE(memchr("\xFF", -1, 1) != NULL);

    _TEST_INT(offset(word, strchr(word, 'b')), 1);
    _TEST_INT(offset(word, strchr(word, '\0')), 6);
    _TEST_INT(offset(word, strchr(word, 'z')), NONE);
    _TEST_INT(offset(word, strchr(word, 0x100 + 'c')), 2);
    _TEST_TRUE(strchr("\xFF", -1) != NULL);

    _TEST_UINT(strcspn("abcde", "dc"), 2);
    _TEST_UINT(strcspn("abc", ""), 3);
    _TEST_UINT(strcspn("", "a"), 0);
    _TEST_UINT(strcspn("abc", "xyz"), 3);

    _TEST_INT(offset(word, strpbrk(word, "cb")), 1);
    _TEST_INT(offset(word, strpbrk(word, "xyz")), NONE);
    _TEST_INT(offset(word, strpbrk(word, "")), NONE);

    _TEST_INT(offset(word, strrchr(word, 'b')), 4);
    _TEST_INT(offset(word, strrchr(word, '\0')), 6);
    _TEST_INT(offset(word, strrchr(word, 'z')), NONE);

    _TEST_UINT(strspn("aabbc", "ab"), 4);
    _TEST_UINT(strspn("abc", ""), 0);
    _TEST_UINT(strspn("", "a"), 0);
    _TEST_UINT(strspn("abc", "cba"), 3);

    _TEST_INT(offset(word, strstr(word, "ca")), 2);
    _TEST_INT(offset(word, strstr(word, "")), 0);
    _TEST_INT(offset(word, strstr(word, "abd")), NONE);
    _TEST_INT(offset(word, strstr(word, "abcabcd")), NONE);
    _TEST_TRUE(strstr("aaaab", "aaab") != NULL);
    _TEST_TRUE(strstr("ab", "abc") == NULL);
    _TEST_TRUE(strstr("", "") != NULL);

    _TEST_INT(offset(text, strtok(text, "?")), 1);
    _TEST_MEM(text + 1, "a", 2);
    _TEST_INT(offset(text, strtok(NULL, ",")), 3);
    _TEST_MEM(text + 3, "??b", 4);
    _TEST_INT(offset(text, strtok(NULL, "#,")), 10);
    _TEST_MEM(text + 10, "c", 2);
    _TEST_INT(offset(text, strtok(NULL, "?")), NONE);
    put(buf, ",,,", 4);
    _TEST_INT(offset(buf, strtok(buf, ",")), NONE);
    _TEST_INT(offset(buf, strtok(NULL, ",")), NONE);
}

// memset fills, strerror names an error number and strlen counts to the null.
static void Test_StringMiscellaneous(void)
{
    char buf[SIZE];

    put(buf, "xxxxxxxx", FILLED);
    _TEST_TRUE(memset(buf, 'a', 3) == buf);
    _TEST_MEM(buf, "aaaxxxxx", FILLED);
    _TEST_TRUE(memset(buf, 0x100 + 'b', 2) == buf);
    _TEST_MEM(buf, "bbaxxxxx", FILLED);
    _TEST_TRUE(memset(buf, 'c', 0) == buf);
    _TEST_MEM(buf, "bbaxxxxx", FILLED);

    _TEST_STR(strerror(EDOM), "Numerical argument out of domain");
    _TEST_STR(strerror(EILSEQ), "Invalid or incomplete multibyte or wide character");
    _TEST_STR(strerror(ERANGE), "Numerical result out of range");
    _TEST_STR(strerror(0), "Success");
    _TEST_STR(strerror(1000), "Unknown error 1000");
    _TEST_STR(strerror(-1), "Unknown error -1");
    _TEST_STR(strerror(INT_MIN), "Unknown error -2147483648");

    _TEST_UINT(strlen(""), 0);
    _TEST_UINT(strlen("abc"), 3);
    _TEST_UINT(strlen("ab\0c"), 2);
}

// A pointer reaches each function, and a call in parentheses any macro's.
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
    Test_StringCopying();
    Test_StringConcatenation();
    Test_StringComparison();
    Test_StringSearch();
    Test_StringMiscellaneous();
    Test_StringCalls();
    return _Test_Status();
}
