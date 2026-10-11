// (Test) Status: 0
// A test file for <stdlib.h> rand and srand.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define DRAWS 50

static int draws[DRAWS];

// (S7.20.2.1p2) rand should draw within its range: return the first place out.
static int draw(void)
{
    int i;
    int out = NONE;

    for (i = 0; i < DRAWS; i++) {
        draws[i] = rand();
        if (out == NONE && (draws[i] < 0 || draws[i] > RAND_MAX)) {
            out = i;
        }
    }
    return out;
}

// (S7.20.2.2p2) rand should draw the same again: return the first place off.
static int redrawn(void)
{
    int i;
    int out = NONE;

    for (i = 0; i < DRAWS; i++) {
        if ((rand)() != draws[i] && out == NONE) {
            out = i;
        }
    }
    return out;
}

// (S7.20.2.2p2) rand should draw within its range, and again after its seed.
static void Test_StdlibRand(void)
{
    _TEST_INT(draw(), NONE);
    srand(1);
    _TEST_INT(redrawn(), NONE);
    (srand)(5);
    _TEST_INT(draw(), NONE);
    srand(5);
    _TEST_INT(redrawn(), NONE);
}

int main(void)
{
    Test_StdlibRand();
    return _Test_Status();
}
