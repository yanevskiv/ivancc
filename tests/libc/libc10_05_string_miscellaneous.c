// (Test) Status: 0
// A test file for <string.h> miscellaneous functions.

// Include helpers.
#include "libc10_00_string_helpers.inc"

// [C99 S7.21.6] memset should fill, strerror name a number, strlen count to null.
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

int main(void)
{
    Test_StringMiscellaneous();
    return _Test_Status();
}
