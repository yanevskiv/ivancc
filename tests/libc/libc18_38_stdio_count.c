// (Test) Status: 0
// A test file for <stdio.h> %n.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.1p8) %n should store the characters written so far, converting none.
static void Test_StdioCount(void)
{
    int n = NONE;
    int m = NONE;
    long ln = NONE;
    short hn = NONE;
    size_t zn = 0;
    char text[HOLD];
    intmax_t jn = NONE;
    ptrdiff_t tn = NONE;
    long long lln = NONE;
    signed char hhn = NONE;
    signed char pair[2] = {NONE, NONE};

    _TEST_INT(snprintf(text, sizeof(text), "%n", &n), 0);
    _TEST_INT(n, 0);
    _TEST_STR(text, "");
    _TEST_INT(snprintf(text, sizeof(text), "abc%n", &n), 3);
    _TEST_INT(n, 3);
    _TEST_STR(text, "abc");
    _TEST_INT(snprintf(text, sizeof(text), "abc%ndef", &n), 6);
    _TEST_INT(n, 3);
    _TEST_STR(text, "abcdef");
    _TEST_INT(snprintf(text, sizeof(text), "%d%n%s%n", 12345, &n, "ab", &m), 7);
    _TEST_INT(n, 5);
    _TEST_INT(m, 7);
    _TEST_STR(text, "12345ab");
    _TEST_INT(snprintf(text, sizeof(text), "%5d%n|%-4s%n|", 1, &n, "a", &m), 11);
    _TEST_INT(n, 5);
    _TEST_INT(m, 10);

    _TEST_INT(snprintf(text, SMALL / 4, "abcdefgh%n", &n), 8);
    _TEST_INT(n, 8);
    _TEST_STR(text, "abc");
    _TEST_INT(snprintf(NULL, 0, "%20d%n", 1, &n), 20);
    _TEST_INT(n, 20);

    _TEST_INT(snprintf(text, sizeof(text), "ab%hhn", &hhn), 2);
    _TEST_INT(hhn, 2);
    _TEST_INT(snprintf(text, sizeof(text), "abc%hn", &hn), 3);
    _TEST_INT(hn, 3);
    _TEST_INT(snprintf(text, sizeof(text), "abcd%ln", &ln), 4);
    _TEST_INT(ln, 4);
    _TEST_INT(snprintf(text, sizeof(text), "abcde%lln", &lln), 5);
    _TEST_INT(lln, 5);
    _TEST_INT(snprintf(text, sizeof(text), "abcdef%jn", &jn), 6);
    _TEST_INT(jn, 6);
    _TEST_INT(snprintf(text, sizeof(text), "abcdefg%zn", &zn), 7);
    _TEST_UINT(zn, 7);
    _TEST_INT(snprintf(text, sizeof(text), "abcdefgh%tn", &tn), 8);
    _TEST_INT(tn, 8);

    _TEST_INT(snprintf(text, sizeof(text), "a%hhn", &pair[0]), 1);
    _TEST_INT(pair[0], 1);
    _TEST_INT(pair[1], NONE);
}

int main(void)
{
    Test_StdioCount();
    return _Test_Status();
}
