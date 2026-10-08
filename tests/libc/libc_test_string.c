// (Test) Status: 0
// <string.h> declares the copying, concatenation, comparison, search and miscellaneous functions, 7.21.

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

static void put(char *to, const char *from, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        to[i] = from[i];
    }
}

static bool same(const char *got, const char *want, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (got[i] != want[i]) return false;
    }
    return true;
}

static int sign(int value)
{
    return (value > 0) - (value < 0);
}

int main(void)
{
    static char text[] = "?a???b,,,#c";
    const char *bytes = "ab\0cab";
    const char *word = "abcabc";
    void *(*copy)(void *restrict, const void *restrict, size_t) = memcpy;
    size_t (*length)(const char *) = strlen;
    char buf[32];
    char other[32];
    size_t len;

    put(buf, "xxxxxxxx", 9);
    if (memcpy(buf, "ab\0cd", 5) != buf or not same(buf, "ab\0cdxxx", 9)) return 1;
    if (memcpy(buf, "z", 0) != buf or not same(buf, "ab\0cdxxx", 9)) return 1;

    put(buf, "abcdefgh", 9);
    if (memmove(buf + 2, buf, 5) != buf + 2 or not same(buf, "ababcdeh", 9)) return 2;
    put(buf, "abcdefgh", 9);
    if (memmove(buf, buf + 2, 5) != buf or not same(buf, "cdefgfgh", 9)) return 2;

    put(buf, "xxxxxxxx", 9);
    if (strcpy(buf, "abc") != buf or not same(buf, "abc\0xxxx", 9)) return 3;
    if (strcpy(buf, "") != buf or not same(buf, "\0bc\0xxxx", 9)) return 3;

    put(buf, "xxxxxxxx", 9);
    if (strncpy(buf, "ab", 5) != buf or not same(buf, "ab\0\0\0xxx", 9)) return 4;
    if (strncpy(buf, "abcdefg", 3) != buf or not same(buf, "abc\0\0xxx", 9)) return 4;

    put(buf, "ab\0xxxxx", 9);
    if (strcat(buf, "cd") != buf or not same(buf, "abcd\0xxx", 9)) return 5;
    if (strcat(buf, "") != buf or not same(buf, "abcd\0xxx", 9)) return 5;

    put(buf, "ab\0xxxxx", 9);
    if (strncat(buf, "cdef", 2) != buf or not same(buf, "abcd\0xxx", 9)) return 6;
    if (strncat(buf, "e", 5) != buf or not same(buf, "abcde\0xx", 9)) return 6;

    if (memcmp("abc", "abd", 3) >= 0 or memcmp("abd", "abc", 3) <= 0 or memcmp("abc", "abd", 2) != 0) return 7;
    if (memcmp("ab\0c", "ab\0c", 4) != 0 or memcmp("a", "b", 0) != 0 or memcmp("\x80", "\x01", 1) <= 0) return 7;

    if (strcmp("abc", "abc") != 0 or strcmp("abc", "abd") >= 0 or strcmp("abd", "abc") <= 0) return 8;
    if (strcmp("ab", "abc") >= 0 or strcmp("abc", "ab") <= 0 or strcmp("", "") != 0 or strcmp("\x80", "a") <= 0) return 8;

    if (sign(strcoll("abc", "abd")) != -1 or sign(strcoll("b", "a")) != 1 or strcoll("abc", "abc") != 0) return 9;
    if (sign(strcoll("ab", "abc")) != -1 or sign(strcoll("\x80", "a")) != 1) return 9;

    if (strncmp("abcx", "abcy", 3) != 0 or strncmp("abcx", "abcy", 4) >= 0 or strncmp("ab", "abc", 5) >= 0) return 10;
    if (strncmp("ab\0x", "ab\0y", 4) != 0 or strncmp("a", "b", 0) != 0 or strncmp("\x80", "a", 1) <= 0) return 10;

    len = strxfrm(buf, "abd", sizeof(buf));
    if (len >= sizeof(buf) or buf[len] != '\0' or strxfrm(NULL, "abd", 0) != len) return 11;
    len = strxfrm(other, "abc", sizeof(other));
    if (len >= sizeof(other) or sign(strcmp(other, buf)) != sign(strcoll("abc", "abd"))) return 11;

    if (memchr(bytes, 'b', 6) != bytes + 1 or memchr(bytes, 'c', 6) != bytes + 3 or memchr(bytes, '\0', 6) != bytes + 2) return 12;
    if (memchr(bytes, 'c', 3) != NULL or memchr(bytes, 'z', 6) != NULL or memchr(bytes, 0x100 + 'b', 6) != bytes + 1) return 12;
    if (memchr("\xFF", -1, 1) == NULL) return 12;

    if (strchr(word, 'b') != word + 1 or strchr(word, '\0') != word + 6 or strchr(word, 'z') != NULL) return 13;
    if (strchr(word, 0x100 + 'c') != word + 2 or strchr("\xFF", -1) == NULL) return 13;

    if (strcspn("abcde", "dc") != 2 or strcspn("abc", "") != 3 or strcspn("", "a") != 0 or strcspn("abc", "xyz") != 3) return 14;

    if (strpbrk(word, "cb") != word + 1 or strpbrk(word, "xyz") != NULL or strpbrk(word, "") != NULL) return 15;

    if (strrchr(word, 'b') != word + 4 or strrchr(word, '\0') != word + 6 or strrchr(word, 'z') != NULL) return 16;

    if (strspn("aabbc", "ab") != 4 or strspn("abc", "") != 0 or strspn("", "a") != 0 or strspn("abc", "cba") != 3) return 17;

    if (strstr(word, "ca") != word + 2 or strstr(word, "") != word or strstr(word, "abd") != NULL) return 18;
    if (strstr("aaaab", "aaab") == NULL or strstr("ab", "abc") != NULL or strstr("", "") == NULL) return 18;

    if (strtok(text, "?") != text + 1 or not same(text + 1, "a", 2)) return 19;
    if (strtok(NULL, ",") != text + 3 or not same(text + 3, "??b", 4)) return 19;
    if (strtok(NULL, "#,") != text + 10 or not same(text + 10, "c", 2)) return 19;
    if (strtok(NULL, "?") != NULL) return 19;
    put(buf, ",,,", 4);
    if (strtok(buf, ",") != NULL or strtok(NULL, ",") != NULL) return 19;

    put(buf, "xxxxxxxx", 9);
    if (memset(buf, 'a', 3) != buf or not same(buf, "aaaxxxxx", 9)) return 20;
    if (memset(buf, 0x100 + 'b', 2) != buf or not same(buf, "bbaxxxxx", 9)) return 20;
    if (memset(buf, 'c', 0) != buf or not same(buf, "bbaxxxxx", 9)) return 20;

    if (strcmp(strerror(EDOM), "Numerical argument out of domain") != 0) return 21;
    if (strcmp(strerror(EILSEQ), "Invalid or incomplete multibyte or wide character") != 0) return 21;
    if (strcmp(strerror(ERANGE), "Numerical result out of range") != 0) return 21;
    if (strcmp(strerror(0), "Success") != 0) return 21;
    if (strcmp(strerror(1000), "Unknown error 1000") != 0) return 21;
    if (strcmp(strerror(-1), "Unknown error -1") != 0) return 21;
    if (strcmp(strerror(INT_MIN), "Unknown error -2147483648") != 0) return 21;

    if (strlen("") != 0 or strlen("abc") != 3 or strlen("ab\0c") != 2) return 22;

    if (copy(buf, "zy", 3) != buf or not same(buf, "zy", 3) or length("abcd") != 4) return 23;
    if ((strlen)("abcd") != 4 or (strcmp)("a", "a") != 0 or (memset)(buf, 'q', 1) != buf or buf[0] != 'q') return 23;
    return 0;
}
