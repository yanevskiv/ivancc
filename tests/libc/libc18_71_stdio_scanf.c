// (Test) Status: 0
// (Test) Input:
// | first second
// |    key = some value
// | rest of the line
// | 50% done
// A test file for <stdio.h> scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.4p2] scanf should read standard input as fscanf reads a stream.
static void Test_StdioScanf(void)
{
    int n = NONE;
    char ch = FILL;
    char line[LINE];
    char first[SMALL];
    char second[SMALL];

    _TEST_INT(scanf("%s%s", first, second), 2);
    _TEST_STR(first, "first");
    _TEST_STR(second, "second");
    _TEST_INT(getchar(), '\n');
    _TEST_INT(scanf("%s = %[^\n]%c", first, second, &ch), 3);
    _TEST_STR(first, "key");
    _TEST_STR(second, "some value");
    _TEST_INT(ch, '\n');
    _TEST_INT(scanf("%4c%n", line, &n), 1);
    _TEST_MEM(line, "rest", 4);
    _TEST_INT(n, 4);
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == line);
    _TEST_STR(line, " of the line\n");
    _TEST_INT(scanf("50%% %s", first), 1);
    _TEST_STR(first, "done");
    _TEST_INT(scanf("%s", first), EOF);
    _TEST_TRUE(feof(stdin));
}

int main(void)
{
    Test_StdioScanf();
    return _Test_Status();
}
