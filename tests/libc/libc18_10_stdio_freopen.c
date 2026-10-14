// (Test) Status: 0
// A test file for <stdio.h> freopen.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.5.4p2] freopen should give a stream another file, indicators cleared.
static void Test_StdioFreopen(void)
{
    FILE *file = fopen("file", "w");
    char line[LINE];

    if (! _TEST_INT(getchar(), EOF)) {
        return;
    }
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("first", file) >= 0);
    if (! _TEST_TRUE(freopen("other", "w", file) == file)) {
        return;
    }
    _TEST_TRUE(fputs("second", file) >= 0);
    if (! _TEST_TRUE(freopen("file", "r", file) == file)) {
        return;
    }
    _TEST_TRUE(reads(file, "first"));
    _TEST_TRUE(feof(file));
    if (! _TEST_TRUE(freopen("other", "a+", file) == file)) {
        return;
    }
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_TRUE(fputs("!", file) >= 0);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("other", "second!"));

    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fgetc(file), 'f');
    _TEST_INT(ungetc('?', file), '?');
    if (! _TEST_TRUE(freopen("other", "r", file) == file)) {
        return;
    }
    _TEST_TRUE(reads(file, "second!"));
    _TEST_TRUE(freopen("missing", "r", file) == NULL);

    _TEST_TRUE(feof(stdin));
    if (! _TEST_TRUE(freopen("other", "r", stdin) == stdin)) {
        return;
    }
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(getchar(), 's');
    _TEST_TRUE(gets(line) == line);
    _TEST_STR(line, "econd!");
}

int main(void)
{
    Test_StdioFreopen();
    return _Test_Status();
}
