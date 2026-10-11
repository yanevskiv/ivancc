// Includable helper file for <setjmp.h> tests.

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
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>

// Defines.
#define RETURNED (-1)
#define LANDED   10
#define BUFS     3

// Types.
struct Holder {
    char ho_tag;
    jmp_buf ho_env;
    int ho_after;
};

// Objects.
static jmp_buf env;
static volatile long seed = 1;

// Jump to a buffer unless seed is 0, so gcc keeps every frame that leads here.
static void leap(jmp_buf to, int val)
{
    if (seed != 0) {
        longjmp(to, val);
    }
}

// Jump to a buffer with a value from depth calls down.
static void jump(jmp_buf to, int depth, int val)
{
    if (depth > 0) {
        jump(to, depth - 1, val);
    }
    leap(to, val);
}

// Step a count and, the first time, jump with a value from depth calls down.
static void once(volatile int *count, jmp_buf to, int depth, int val)
{
    (*count)++;
    if (*count == 1) {
        jump(to, depth, val);
        *count = RETURNED;
    }
}

// Step a count and, up to a limit, jump with the count from depth calls down.
static void each(volatile int *count, jmp_buf to, int depth, int limit)
{
    (*count)++;
    if (*count <= limit) {
        jump(to, depth, *count);
        *count = RETURNED;
    }
}
