// (Test) Status: 0
// A test file for <stdio.h> fscanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define TEXT "one two\n" LONGER "x%y\nac"

static char filebuf[SMALL];

// fscanf should leave unread what follows each item it reads.
// [C99:7.19.6.2p9]: an input item, the longest prefix of a matching sequence
static bool feeds(int kind)
{
    FILE *file;
    int lost = 0;
    char ch = FILL;
    char first[HOLD];
    char second[HOLD];
    char *buf = kind == _IONBF ? NULL : filebuf;

    if (! put("file", "w", TEXT)) {
        return false;
    }
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_INT(fscanf(file, "%s%s", first, second), 2);
    lost += ! _TEST_STR(first, "one");
    lost += ! _TEST_STR(second, "two");
    lost += ! _TEST_INT(fgetc(file), '\n');
    lost += ! _TEST_INT(fscanf(file, "%[^\n]", first), 1);
    lost += ! _TEST_MEM(first, LONGER, strlen(LONGER) - 1);
    lost += ! _TEST_INT(fscanf(file, " x%%%c", &ch), 1);
    lost += ! _TEST_INT(ch, 'y');
    lost += ! _TEST_INT(ungetc('q', file), 'q');
    lost += ! _TEST_INT(fscanf(file, "%c", &ch), 1);
    lost += ! _TEST_INT(ch, 'q');
    lost += ! _TEST_INT(ungetc('r', file), 'r');
    lost += ! _TEST_INT(fgetc(file), 'r');
    lost += ! _TEST_INT(fscanf(file, " ab"), 0);
    lost += ! _TEST_INT(fgetc(file), 'c');
    lost += ! _TEST_TRUE(! feof(file));
    lost += ! _TEST_INT(fscanf(file, "%s", first), EOF);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_TRUE(! ferror(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// fscanf should read a stream the same however buffered.
// [C99:7.19.6.2p2]: fscanf, its format and the arguments it assigns
static void Test_StdioFscanf(void)
{
    _TEST_TRUE(feeds(_IOFBF));
    _TEST_TRUE(feeds(_IOLBF));
    _TEST_TRUE(feeds(_IONBF));
}

int main(void)
{
    Test_StdioFscanf();
    return _Test_Status();
}
