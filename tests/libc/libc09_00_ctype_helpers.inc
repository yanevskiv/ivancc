// Includable helper file for <ctype.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

// Defines.
#define NONE (-1)

#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER "abcdefghijklmnopqrstuvwxyz"

// Return a character's place in a set, or NONE where the set lacks it.
static int place(const char *set, int ch)
{
    int i;

    for (i = 0; set[i] != '\0'; i++) {
        if ((unsigned char) set[i] == ch) {
            return i;
        }
    }
    return NONE;
}
