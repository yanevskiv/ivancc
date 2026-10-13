// (Test) Status: 0
// A test file for <stdio.h> sscanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.7p2] sscanf should read a string as fscanf reads a stream.
static void Test_StdioSscanf(void)
{
    int n = NONE;
    char first[SMALL];
    char third[SMALL];
    char second[SMALL];

    _TEST_INT(sscanf("one two three", "%s%s%s", first, second, third), 3);
    _TEST_STR(first, "one");
    _TEST_STR(second, "two");
    _TEST_STR(third, "three");
    _TEST_INT(sscanf("key = value", "%s = %s", first, second), 2);
    _TEST_STR(first, "key");
    _TEST_STR(second, "value");
    _TEST_INT(sscanf("a,b", "%[^,],%s%n", first, second, &n), 2);
    _TEST_STR(first, "a");
    _TEST_STR(second, "b");
    _TEST_INT(n, 3);
    _TEST_INT(sscanf("", ""), 0);
    _TEST_INT(sscanf("left over", ""), 0);
    _TEST_INT(sscanf("abc", "abc"), 0);
    _TEST_INT(sscanf("word", "%s", first, second), 1);
    _TEST_STR(first, "word");
    _TEST_STR(second, "b");
}

int main(void)
{
    Test_StdioSscanf();
    return _Test_Status();
}
