// Includable helper file for <stddef.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>

// Defines.
#define TAIL 3

// Types.
struct Inner {
    char in_first;
    long in_second;
};

struct Pair {
    char pa_ch;
    int pa_number;
    char pa_tail[TAIL];
    struct Inner pa_inner;
};
