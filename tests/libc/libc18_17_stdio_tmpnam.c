// (Test) Status: 0
// A test file for <stdio.h> tmpnam.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

static char names[NAMES][L_tmpnam];

// (S7.19.4.4p3) tmpnam should give NAMES new names: return the first not new.
static int unnamed(void)
{
    int k;
    int n;
    const char *got;

    for (k = 0; k < NAMES; k++) {
        got = tmpnam(k % 2 == 0 ? names[k] : NULL);
        if (got == NULL || strlen(got) >= L_tmpnam || (k % 2 == 0 && got != names[k])) {
            return k;
        }
        if (k % 2 != 0) {
            strcpy(names[k], got);
        }
        if (fopen(names[k], "r") != NULL) {
            return k;
        }
        for (n = 0; n < k; n++) {
            if (strcmp(names[n], names[k]) == 0) {
                return k;
            }
        }
    }
    return NONE;
}

// (S7.19.4.4p3) tmpnam should give a new name each call, one a file can take.
static void Test_StdioTmpnam(void)
{
    _TEST_INT(unnamed(), NONE);
    _TEST_TRUE(put(names[0], "w", "named"));
    _TEST_TRUE(holds(names[0], "named"));
    _TEST_INT(remove(names[0]), 0);
}

int main(void)
{
    Test_StdioTmpnam();
    return _Test_Status();
}
