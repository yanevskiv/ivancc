// (Test) Status: 0
// A test file for <wchar.h> reading characters of a wide string.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

// (S7.24.2.2p12) %c, %s and %[ should store wide characters as bytes.
static void Test_WcharSwscanfBytes(void)
{
    int n = NONE;
    char out[SIZE];
    char next[SIZE];

    memset(out, 'x', sizeof(out));
    _TEST_INT(swscanf(L"abc", L"%c", out), 1);
    _TEST_MEM(out, "axx", 3);
    _TEST_INT(swscanf(L" bcd", L"%3c%n", out, &n), 1);
    _TEST_MEM(out, " bcx", 4);
    _TEST_INT(n, 3);
    _TEST_INT(swscanf(L"  word next", L"%s%s%n", out, next, &n), 2);
    _TEST_STR(out, "word");
    _TEST_STR(next, "next");
    _TEST_INT(n, 11);
    _TEST_INT(swscanf(L"abcdef", L"%2s%3s", out, next), 2);
    _TEST_STR(out, "ab");
    _TEST_STR(next, "cde");
    _TEST_INT(swscanf(L"abc123", L"%[a-c]%[0-9]", out, next), 2);
    _TEST_STR(out, "abc");
    _TEST_STR(next, "123");
    _TEST_INT(swscanf(L"a]b-c;", L"%[]ab]%[^;]", out, next), 2);
    _TEST_STR(out, "a]b");
    _TEST_STR(next, "-c");
    _TEST_INT(swscanf(L" x", L"%[a-z]", out), 0);
    _TEST_INT(swscanf(L"", L"%s", out), EOF);
    _TEST_INT(swscanf(L"skip kept", L"%*s%s", out), 1);
    _TEST_STR(out, "kept");
}

// (S7.24.2.2p11) An l before c, s or [ should store wide characters.
static void Test_WcharSwscanfWide(void)
{
    int n = NONE;
    wchar_t out[SIZE];
    wchar_t next[SIZE];

    put(out, L"xxxxxxxx", FILLED);
    _TEST_INT(swscanf(L"abc", L"%lc", out), 1);
    _TEST_MEM(out, L"axx", WIDE(3));
    _TEST_INT(swscanf(L" bcd", L"%3lc%n", out, &n), 1);
    _TEST_MEM(out, L" bcx", WIDE(4));
    _TEST_INT(n, 3);
    _TEST_INT(swscanf(L"  word next", L"%ls%ls", out, next), 2);
    _TEST_MEM(out, L"word", WIDE(5));
    _TEST_MEM(next, L"next", WIDE(5));
    _TEST_INT(swscanf(L"abcdef", L"%2ls%3ls", out, next), 2);
    _TEST_MEM(out, L"ab", WIDE(3));
    _TEST_MEM(next, L"cde", WIDE(4));
    _TEST_INT(swscanf(L"abc123", L"%l[a-c]%l[^a-c]", out, next), 2);
    _TEST_MEM(out, L"abc", WIDE(4));
    _TEST_MEM(next, L"123", WIDE(4));
    _TEST_INT(swscanf(L"abcd", L"%2l[a-z]%n", out, &n), 1);
    _TEST_MEM(out, L"ab", WIDE(3));
    _TEST_INT(n, 2);
    _TEST_INT(swscanf(L"skip kept", L"%*ls%ls", out), 1);
    _TEST_MEM(out, L"kept", WIDE(5));
}

int main(void)
{
    Test_WcharSwscanfBytes();
    Test_WcharSwscanfWide();
    return _Test_Status();
}
