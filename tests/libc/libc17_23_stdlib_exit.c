// (Test) Status: 0
// A test file for <stdlib.h> atexit and exit.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define EXITS   32
#define MIDDLES (EXITS - 3)
#define EXITED  60

static int stage;
static int middles;

// Registered first, this should run last, which finish prevents.
// [C99:7.20.4.3p3]: exit's first step, atexit's functions in reverse order
static void skipped(void)
{
    int reached = 1;

    _TEST_INT(reached, 0);
    _Exit(_Test_Status());
}

// Registered second, this should run once each later one has.
// [C99:7.20.4.3p3]: exit's first step, atexit's functions in reverse order
static void finish(void)
{
    _TEST_INT(stage, 1);
    _TEST_INT(middles, MIDDLES);
    _Exit(_Test_Status());
}

// Registered between them, this should run once last has.
// [C99:7.20.4.3p3]: exit's first step, atexit's functions in reverse order
static void middle(void)
{
    if (stage == 1) {
        middles++;
    }
}

// Registered last, this should run first.
// [C99:7.20.4.3p3]: exit's first step, atexit's functions in reverse order
static void last(void)
{
    if (stage == 0 && middles == 0) {
        stage = 1;
    }
}

// atexit should take 32 functions: return the first it refuses.
// [C99:7.20.4.2p3]: at least 32 functions registered by atexit
static int unregistered(void)
{
    int i;

    for (i = 0; i < MIDDLES; i++) {
        if (atexit(middle) != 0) {
            return i;
        }
    }
    return NONE;
}

// exit should run the functions registered, last first.
// [C99:7.20.4.3p3]: exit's first step, atexit's functions in reverse order
static void Test_StdlibExit(void)
{
    int returned = 0;

    _TEST_INT(atexit(skipped), 0);
    _TEST_INT(atexit(finish), 0);
    _TEST_INT(unregistered(), NONE);
    _TEST_INT((atexit)(last), 0);
    (exit)(EXITED);
    returned = 1;
    _TEST_INT(returned, 0);
}

int main(void)
{
    Test_StdlibExit();
    return _Test_Status();
}
