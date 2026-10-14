// (Test) Status: 0
// A test file for <stdlib.h> blocks that come and go.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define STEPS    1500
#define RARELY   4
#define LONGEST  2000
#define SHORTEST 100
#define PIECE    65536
#define GROWTH   16
#define SCATTER  7
#define ROUNDS   200

static size_t length[SLOTS];

// Blocks made, resized and freed at random should keep their bytes.
// [C99:7.20.3p1]: allocated storage, suitably aligned
static bool churn(void)
{
    size_t i;
    size_t k;
    size_t n;
    long step;
    size_t kept;
    unsigned char *ptr;

    for (step = 0; step < STEPS; step++) {
        k = next() % SLOTS;
        n = next() % RARELY == 0 ? next() % LONGEST : next() % SHORTEST;
        if (slot[k] != NULL && next() % 2 == 0) {
            if (! _TEST_INT(spoiled(slot[k], length[k], (unsigned char) k), NONE)) {
                return false;
            }
            free(slot[k]);
            slot[k] = NULL;
        } else {
            kept = slot[k] == NULL ? 0 : length[k] < n + 1 ? length[k] : n + 1;
            ptr = slot[k] != NULL ? realloc(slot[k], n + 1) : malloc(n + 1);
            if (! _TEST_TRUE(ptr != NULL) || ! _TEST_UINT(misaligned(ptr), 0)) {
                return false;
            }
            if (! _TEST_INT(spoiled(ptr, kept, (unsigned char) k), NONE)) {
                return false;
            }
            fill(ptr, n + 1, (unsigned char) k);
            slot[k] = ptr;
            length[k] = n + 1;
        }
    }
    for (i = 0; i < SLOTS; i++) {
        if (slot[i] != NULL && ! _TEST_INT(spoiled(slot[i], length[i], (unsigned char) i), NONE)) {
            return false;
        }
        free(slot[i]);
        slot[i] = NULL;
    }
    return true;
}

// Freed small blocks should merge to hold a large one, 200 times over.
static bool merges(void)
{
    int i;
    int round;
    size_t piece;
    unsigned char *large;
    unsigned char *small[SMALL];

    for (round = 0; round < ROUNDS; round++) {
        piece = PIECE + (size_t) round * GROWTH;
        for (i = 0; i < SMALL; i++) {
            small[i] = malloc(piece);
            if (! _TEST_TRUE(small[i] != NULL)) {
                return false;
            }
        }
        for (i = 0; i < SMALL; i++) {
            free(small[(i * SCATTER) % SMALL]);
        }
        large = malloc(SMALL * piece);
        if (! _TEST_TRUE(large != NULL)) {
            return false;
        }
        large[0] = 1;
        large[SMALL * piece - 1] = 1;
        free(large);
    }
    return true;
}

// Live blocks should keep their bytes, and freed ones merge.
// [C99:7.20.3p1]: allocated storage, suitably aligned
static void Test_StdlibHeap(void)
{
    _TEST_TRUE(churn());
    _TEST_TRUE(merges());
}

int main(void)
{
    Test_StdlibHeap();
    return _Test_Status();
}
