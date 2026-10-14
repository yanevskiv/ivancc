// (Test) Status: 0
// A test file for <stdio.h> %p.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %p should write a pointer as printing characters.
// [C99:7.19.6.1p8]: printf's conversion specifiers
static bool shows(const void *ptr, char *text, size_t size)
{
    int n;
    int lost = 0;
    size_t k;

    n = snprintf(text, size, "%p", ptr);
    lost += ! _TEST_INT(n, (int) strlen(text));
    for (k = 0; text[k] != '\0'; k++) {
        lost += ! _TEST_TRUE(isprint((unsigned char) text[k]));
    }
    return lost == 0;
}

// %p should tell two pointers apart, for fscanf to read back.
// [C99:7.19.6.2p12]: scanf's conversion specifiers
static void Test_StdioPointer(void)
{
    int one = 0;
    int two = 0;
    char null[HOLD];
    char first[HOLD];
    char again[HOLD];
    char second[HOLD];
    char padded[HOLD];

    _TEST_TRUE(shows(&one, first, sizeof(first)));
    _TEST_TRUE(shows(&one, again, sizeof(again)));
    _TEST_TRUE(shows(&two, second, sizeof(second)));
    _TEST_TRUE(shows(NULL, null, sizeof(null)));
    _TEST_TRUE(shows(wide + WIDE, padded, sizeof(padded)));

    _TEST_STR(again, first);
    _TEST_TRUE(strcmp(first, second) != 0);
    _TEST_TRUE(strcmp(first, null) != 0);
    _TEST_TRUE(strcmp(second, null) != 0);

    _TEST_INT(snprintf(padded, sizeof(padded), "%40p", (void *) &one), 40);
    _TEST_INT((int) strlen(padded), 40);
    _TEST_STR(padded + 40 - strlen(first), first);
    _TEST_INT(padded[0], ' ');
    _TEST_INT(snprintf(padded, sizeof(padded), "%-40p|", (void *) &one), 41);
    _TEST_MEM(padded, first, strlen(first));
    _TEST_INT(padded[39], ' ');
    _TEST_INT(padded[40], '|');
}

int main(void)
{
    Test_StdioPointer();
    return _Test_Status();
}
