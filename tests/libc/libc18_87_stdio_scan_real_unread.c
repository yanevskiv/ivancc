// (Test) Status: 0
// A test file for <stdio.h> what fscanf leaves unread after a floating number.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define NINES "9999999999999999999999999999999999999999999999999999999999999999"

static char filebuf[SMALL];

// The first character after a number's item should stay unread.
// [C99:7.19.6.2p9]: an input item, the longest prefix of a matching sequence
static bool leaves(int kind)
{
    FILE *file;
    int lost = 0;
    float value = NONE;
    double wide = NONE;
    char *buf = kind == _IONBF ? NULL : filebuf;

    if (! put("file", "w", "1.5ab -x 1..5 1e5e5 0x1p2g infinityx nan(1)z ." NINES NINES "e-126,\n")) {
        return false;
    }
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_INT(fscanf(file, "%f", &value), 1);
    lost += ! _TEST_FLOAT(value, 1.5f);
    lost += ! _TEST_INT(fgetc(file), 'a');
    lost += ! _TEST_INT(fgetc(file), 'b');
    lost += ! _TEST_INT(fscanf(file, "%f", &value), 0);
    lost += ! _TEST_FLOAT(value, 1.5f);
    lost += ! _TEST_INT(fgetc(file), 'x');
    lost += ! _TEST_INT(fscanf(file, "%f", &value), 1);
    lost += ! _TEST_FLOAT(value, 1.0f);
    lost += ! _TEST_INT(ungetc('2', file), '2');
    lost += ! _TEST_INT(fscanf(file, "%f", &value), 1);
    lost += ! _TEST_FLOAT(value, 2.5f);
    lost += ! _TEST_INT(fscanf(file, "%e", &value), 1);
    lost += ! _TEST_FLOAT(value, 100000.0f);
    lost += ! _TEST_INT(fgetc(file), 'e');
    lost += ! _TEST_INT(fgetc(file), '5');
    lost += ! _TEST_INT(fscanf(file, "%a", &value), 1);
    lost += ! _TEST_FLOAT(value, 4.0f);
    lost += ! _TEST_INT(fgetc(file), 'g');
    lost += ! _TEST_INT(fscanf(file, "%lg", &wide), 1);
    lost += ! _TEST_FLOAT(wide, strtod("inf", NULL));
    lost += ! _TEST_INT(fgetc(file), 'x');
    lost += ! _TEST_INT(fscanf(file, "%lf", &wide), 1);
    lost += ! _TEST_FLOAT(wide, strtod("nan", NULL));
    lost += ! _TEST_INT(fgetc(file), 'z');
    lost += ! _TEST_INT(fscanf(file, "%lf", &wide), 1);
    lost += ! _TEST_FLOAT(wide, 1e-126);
    lost += ! _TEST_INT(fgetc(file), ',');
    lost += ! _TEST_TRUE(! feof(file));
    lost += ! _TEST_INT(fscanf(file, "%f", &value), EOF);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// A number's item should end alike however a stream is buffered.
// [C99:7.19.6.2p9]: an input item, the longest prefix of a matching sequence
static void Test_StdioScanRealUnread(void)
{
    _TEST_TRUE(leaves(_IOFBF));
    _TEST_TRUE(leaves(_IOLBF));
    _TEST_TRUE(leaves(_IONBF));
}

int main(void)
{
    Test_StdioScanRealUnread();
    return _Test_Status();
}
