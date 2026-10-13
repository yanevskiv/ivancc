// (Test) Status: 0
// A test file for <stdio.h> what fscanf leaves unread after an integer.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

static char filebuf[SMALL];

// [C99 S7.19.6.2p9] The first character after an integer's item should stay unread.
static bool leaves(int kind)
{
    FILE *file;
    int lost = 0;
    int value = NONE;
    unsigned int bits = FILL;
    char *buf = kind == _IONBF ? NULL : filebuf;

    if (! put("file", "w", "12ab -x 0x1fg 077 8\n")) {
        return false;
    }
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_INT(fscanf(file, "%d", &value), 1);
    lost += ! _TEST_INT(value, 12);
    lost += ! _TEST_INT(fgetc(file), 'a');
    lost += ! _TEST_INT(fgetc(file), 'b');
    lost += ! _TEST_INT(fscanf(file, "%d", &value), 0);
    lost += ! _TEST_INT(value, 12);
    lost += ! _TEST_INT(fgetc(file), 'x');
    lost += ! _TEST_INT(fscanf(file, "%x", &bits), 1);
    lost += ! _TEST_UINT(bits, 0x1f);
    lost += ! _TEST_INT(ungetc('7', file), '7');
    lost += ! _TEST_INT(fscanf(file, "%i", &value), 1);
    lost += ! _TEST_INT(value, 7);
    lost += ! _TEST_INT(fgetc(file), 'g');
    lost += ! _TEST_INT(fscanf(file, "%o", &bits), 1);
    lost += ! _TEST_UINT(bits, 077);
    lost += ! _TEST_INT(fscanf(file, "%o", &bits), 0);
    lost += ! _TEST_INT(fgetc(file), '8');
    lost += ! _TEST_TRUE(! feof(file));
    lost += ! _TEST_INT(fscanf(file, "%u", &bits), EOF);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// [C99 S7.19.6.2p2] fscanf should read integers the same however buffered.
static void Test_StdioScanIntegerUnread(void)
{
    _TEST_TRUE(leaves(_IOFBF));
    _TEST_TRUE(leaves(_IOLBF));
    _TEST_TRUE(leaves(_IONBF));
}

int main(void)
{
    Test_StdioScanIntegerUnread();
    return _Test_Status();
}
