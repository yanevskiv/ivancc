// (Test) Status: 0
// A test file for <wchar.h> wide string search functions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// A search should point at what it finds or nothing, or count.
// [C99:7.24.4.5]: wcschr, wcscspn, wcspbrk, wcsrchr, wcsspn, wcsstr, wcstok and wmemchr
static void Test_WcharSearch(void)
{
    const wchar_t *word = L"abcabc";
    const wchar_t *wides = L"ab\0cab";

    _TEST_INT(offset(word, wcschr(word, L'b')), 1);
    _TEST_INT(offset(word, wcschr(word, L'\0')), 6);
    _TEST_INT(offset(word, wcschr(word, L'z')), NONE);
    _TEST_INT(offset(word, wcschr(word, 0x100 + L'c')), NONE);
    _TEST_TRUE(wcschr(L"a\x20AC", 0x20AC) != NULL);

    _TEST_UINT(wcscspn(L"abcde", L"dc"), 2);
    _TEST_UINT(wcscspn(L"abc", L""), 3);
    _TEST_UINT(wcscspn(L"", L"a"), 0);
    _TEST_UINT(wcscspn(L"abc", L"xyz"), 3);

    _TEST_INT(offset(word, wcspbrk(word, L"cb")), 1);
    _TEST_INT(offset(word, wcspbrk(word, L"xyz")), NONE);
    _TEST_INT(offset(word, wcspbrk(word, L"")), NONE);

    _TEST_INT(offset(word, wcsrchr(word, L'b')), 4);
    _TEST_INT(offset(word, wcsrchr(word, L'\0')), 6);
    _TEST_INT(offset(word, wcsrchr(word, L'z')), NONE);

    _TEST_UINT(wcsspn(L"aabbc", L"ab"), 4);
    _TEST_UINT(wcsspn(L"abc", L""), 0);
    _TEST_UINT(wcsspn(L"", L"a"), 0);
    _TEST_UINT(wcsspn(L"abc", L"cba"), 3);

    _TEST_INT(offset(word, wcsstr(word, L"ca")), 2);
    _TEST_INT(offset(word, wcsstr(word, L"")), 0);
    _TEST_INT(offset(word, wcsstr(word, L"abd")), NONE);
    _TEST_INT(offset(word, wcsstr(word, L"abcabcd")), NONE);
    _TEST_TRUE(wcsstr(L"aaaab", L"aaab") != NULL);
    _TEST_TRUE(wcsstr(L"ab", L"abc") == NULL);
    _TEST_TRUE(wcsstr(L"", L"") != NULL);

    _TEST_INT(offset(wides, wmemchr(wides, L'b', 6)), 1);
    _TEST_INT(offset(wides, wmemchr(wides, L'c', 6)), 3);
    _TEST_INT(offset(wides, wmemchr(wides, L'\0', 6)), 2);
    _TEST_INT(offset(wides, wmemchr(wides, L'c', 3)), NONE);
    _TEST_INT(offset(wides, wmemchr(wides, L'z', 6)), NONE);
    _TEST_INT(offset(wides, wmemchr(wides, L'a', 0)), NONE);
    _TEST_INT(offset(wides, wmemchr(wides, 0x100 + L'b', 6)), NONE);
}

// wcstok should split tokens off, keeping its place in *ptr.
// [C99:7.24.4.5.7]: wcstok
static void Test_WcharTokens(void)
{
    static wchar_t text[] = L"?a???b,,,#c";
    static wchar_t first[] = L"a b";
    static wchar_t second[] = L"c,d";
    wchar_t buf[SIZE];
    wchar_t *place;
    wchar_t *other;

    _TEST_INT(offset(text, wcstok(text, L"?", &place)), 1);
    _TEST_MEM(text + 1, L"a", WIDE(2));
    _TEST_INT(offset(text, wcstok(NULL, L",", &place)), 3);
    _TEST_MEM(text + 3, L"??b", WIDE(4));
    _TEST_INT(offset(text, wcstok(NULL, L"#,", &place)), 10);
    _TEST_MEM(text + 10, L"c", WIDE(2));
    _TEST_INT(offset(text, wcstok(NULL, L"?", &place)), NONE);
    put(buf, L",,,", 4);
    _TEST_INT(offset(buf, wcstok(buf, L",", &place)), NONE);
    _TEST_INT(offset(buf, wcstok(NULL, L",", &place)), NONE);

    _TEST_INT(offset(first, wcstok(first, L" ", &place)), 0);
    _TEST_INT(offset(second, wcstok(second, L",", &other)), 0);
    _TEST_INT(offset(first, wcstok(NULL, L" ", &place)), 2);
    _TEST_INT(offset(second, wcstok(NULL, L",", &other)), 2);
    _TEST_INT(offset(first, wcstok(NULL, L" ", &place)), NONE);
    _TEST_INT(offset(second, wcstok(NULL, L",", &other)), NONE);
}

int main(void)
{
    Test_WcharSearch();
    Test_WcharTokens();
    return _Test_Status();
}
