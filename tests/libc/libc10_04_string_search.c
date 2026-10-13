// (Test) Status: 0
// A test file for <string.h> search functions.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// [C99:S7.21.5] A search should point at what it finds or nothing, and count spans.
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

int main(void)
{
    Test_StringSearch();
    return _Test_Status();
}
