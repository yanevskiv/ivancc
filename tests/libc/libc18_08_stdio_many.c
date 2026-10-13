// (Test) Status: 0
// A test file for <stdio.h> FOPEN_MAX streams open at once.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define MANY (FOPEN_MAX - 3)

// [C99 S7.19.3p15] MANY files should open at once: return the first to fail.
static int crowd(void)
{
    int k;
    char name[] = "many?";
    FILE *many[MANY];

    for (k = 0; k < MANY; k++) {
        name[sizeof(name) - 2] = DIGITS[k];
        many[k] = fopen(name, "w");
        if (many[k] == NULL || fputc(DIGITS[k], many[k]) == EOF) {
            return k;
        }
    }
    for (k = 1; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) {
            return k;
        }
    }
    for (k = 0; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) {
            return k;
        }
    }
    return NONE;
}

// Each of the MANY files should hold its digit: return the first without.
static int uncrowded(void)
{
    int k;
    char name[] = "many?";
    char digit[2];

    for (k = 0; k < MANY; k++) {
        name[sizeof(name) - 2] = DIGITS[k];
        digit[0] = DIGITS[k];
        digit[1] = '\0';
        if (! holds(name, digit)) {
            return k;
        }
    }
    return NONE;
}

// [C99 S7.19.3p15] FOPEN_MAX streams should be open at once, each its own file's.
static void Test_StdioMany(void)
{
    _TEST_INT(crowd(), NONE);
    _TEST_INT(uncrowded(), NONE);
}

int main(void)
{
    Test_StdioMany();
    return _Test_Status();
}
