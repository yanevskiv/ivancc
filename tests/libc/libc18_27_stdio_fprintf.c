// (Test) Status: 0
// A test file for <stdio.h> fprintf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define FIRST "one|  two|3\n"

static char filebuf[SMALL];

// [C99:7.19.6.1p14] fprintf should count what it sends a stream buffered one way.
static bool sends(int kind)
{
    int ch;
    int lost = 0;
    int spaces = 0;
    char *buf = kind == _IONBF ? NULL : filebuf;
    FILE *file = fopen("file", "w");

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_INT(fprintf(file, "%s|%5s|%c\n", "one", "two", '3'), (int) strlen(FIRST));
    lost += ! _TEST_INT(fprintf(file, ""), 0);
    lost += ! _TEST_INT(fprintf(file, "%s", LONGER), (int) strlen(LONGER));
    lost += ! _TEST_INT(fprintf(file, "%*c", WIDE, '\n'), WIDE);
    lost += ! _TEST_TRUE(! ferror(file));
    lost += ! _TEST_INT(fclose(file), 0);
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! reads(file, FIRST);
    lost += ! reads(file, LONGER);
    while ((ch = fgetc(file)) == ' ') {
        spaces++;
    }
    lost += ! _TEST_INT(spaces, WIDE - 1);
    lost += ! _TEST_INT(ch, '\n');
    lost += ! _TEST_INT(fgetc(file), EOF);
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// [C99:7.19.6.1p2] fprintf should write a stream the same however buffered.
static void Test_StdioFprintf(void)
{
    _TEST_TRUE(sends(_IOFBF));
    _TEST_TRUE(sends(_IOLBF));
    _TEST_TRUE(sends(_IONBF));
}

int main(void)
{
    Test_StdioFprintf();
    return _Test_Status();
}
