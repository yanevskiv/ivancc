// (Test) Status: 0
// (Test) Input:
// | abc
// | a line read in two parts
// | gets takes this one
// | 0123456789abcdefghijklmnopqrstuvwxyz
// | a block over a refill
// | end
// A test file for <stdio.h> input through a small buffer.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

static char inbuf[SMALL];

// [C99:7.19.3p3] Each input function should read through a small buffer.
static void Test_StdioInput(void)
{
    char line[LINE];
    char block[HOLD];

    _TEST_INT(setvbuf(stdin, inbuf, _IOFBF, sizeof(inbuf)), 0);
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(getchar(), 'a');
    _TEST_INT(getc(stdin), 'b');
    _TEST_INT(fgetc(stdin), 'c');
    _TEST_INT(ungetc(EOF, stdin), EOF);
    _TEST_INT(ungetc('x' + UCHAR_MAX + 1, stdin), 'x');
    _TEST_INT(fgetc(stdin), 'x');
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_TRUE(fgets(line, 8, stdin) == line);
    _TEST_STR(line, "a line ");
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == line);
    _TEST_STR(line, "read in two parts\n");
    _TEST_TRUE(gets(line) == line);
    _TEST_STR(line, "gets takes this one");
    _TEST_UINT(fread(block, 4, 9, stdin), 9);
    _TEST_MEM(block, DIGITS, 36);
    _TEST_UINT(fread(block, 0, 3, stdin), 0);
    _TEST_UINT(fread(block, 1, 0, stdin), 0);
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_UINT(fread(block, 7, 3, stdin), 3);
    _TEST_MEM(block, "a block over a refill", 21);
    _TEST_INT(ungetc(UCHAR_MAX, stdin), UCHAR_MAX);
    _TEST_INT(fgetc(stdin), UCHAR_MAX);
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_TRUE(! feof(stdin));
    _TEST_UINT(fread(block, 3, 2, stdin), 1);
    _TEST_MEM(block, "end", 3);
    _TEST_TRUE(feof(stdin));
    _TEST_TRUE(! ferror(stdin));

    _TEST_INT(fgetc(stdin), EOF);
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == NULL);
    _TEST_STR(line, "gets takes this one");
    _TEST_TRUE(gets(line) == NULL);
    clearerr(stdin);
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(fgetc(stdin), EOF);
    _TEST_TRUE(feof(stdin));
    _TEST_INT(ungetc('z', stdin), 'z');
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == line);
    _TEST_STR(line, "z");
    _TEST_TRUE(feof(stdin));
}

int main(void)
{
    Test_StdioInput();
    return _Test_Status();
}
