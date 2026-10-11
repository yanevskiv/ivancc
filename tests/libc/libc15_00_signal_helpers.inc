// Includable helper file for <signal.h> tests.

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
#include <assert.h>
#include <time.h>
#include <signal.h>

// Objects.
static volatile sig_atomic_t got;
static volatile sig_atomic_t count;

// Note the signal, and that a handler ran.
static void on_signal(int sig)
{
    got = sig;
    count++;
}

// Note the signal negated, to tell this handler from the other.
static void on_other(int sig)
{
    got = -sig;
}
