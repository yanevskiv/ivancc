// (Test) Status: 0
// <stdlib.h> declares the memory management functions, calloc, free, malloc and realloc (S7.20.3).

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
#include <locale.h>
#include <stdlib.h>

#define SLOTS 100
#define STEPS 1500
#define SMALL 16
#define PIECE 65536
#define ROUNDS 200
#define BIG (1 << 17)

static unsigned char *slot[SLOTS];
static size_t length[SLOTS];
static unsigned long seed = 1;
static volatile size_t huge = SIZE_MAX;

static unsigned long next(void)
{
    seed = seed * 6364136223846793005UL + 1442695040888963407UL;
    return seed >> 33;
}

// The strictest alignment of the basic types, which malloc's memory must meet.
struct strictest {
    char c;
    union {
        long double ld;
        long long ll;
        double d;
        void *ptr;
    } u;
};

static bool aligned(const void *ptr)
{
    return (uintptr_t) ptr % offsetof(struct strictest, u) == 0;
}

static void fill(unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        ptr[i] = (unsigned char) (tag + i);
    }
}

static bool filled(const unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (ptr[i] != (unsigned char) (tag + i)) return false;
    }
    return true;
}

// Allocate, resize and free at random, every live block keeping its own bytes.
static int churn(void)
{
    size_t i;
    long step;

    for (step = 0; step < STEPS; step++) {
        size_t k = next() % SLOTS;
        size_t n = next() % 4 == 0 ? next() % 2000 : next() % 100;
        unsigned char *ptr;

        if (slot[k] and next() % 2 == 0) {
            if (not filled(slot[k], length[k], (unsigned char) k)) return 1;
            free(slot[k]);
            slot[k] = NULL;
            continue;
        }
        ptr = slot[k] ? realloc(slot[k], n + 1) : malloc(n + 1);
        if (ptr == NULL or not aligned(ptr)) return 2;
        if (slot[k] and not filled(ptr, length[k] < n + 1 ? length[k] : n + 1, (unsigned char) k)) return 3;
        fill(ptr, n + 1, (unsigned char) k);
        slot[k] = ptr;
        length[k] = n + 1;
    }
    for (i = 0; i < SLOTS; i++) {
        if (slot[i] and not filled(slot[i], length[i], (unsigned char) i)) return 4;
        free(slot[i]);
        slot[i] = NULL;
    }
    return 0;
}

// Free small blocks and take a large one, each round's a little larger than the last,
// 200 MiB over all: only merged blocks fit them, and without merging the heap runs out.
static bool merges(void)
{
    unsigned char *small[SMALL];
    unsigned char *large;
    size_t piece;
    int round;
    int i;

    for (round = 0; round < ROUNDS; round++) {
        piece = PIECE + (size_t) round * 16;
        for (i = 0; i < SMALL; i++) {
            small[i] = malloc(piece);
            if (small[i] == NULL) return false;
        }
        for (i = 0; i < SMALL; i++) {
            free(small[(i * 7) % SMALL]);
        }
        large = malloc(SMALL * piece);
        if (large == NULL) return false;
        large[0] = 1;
        large[SMALL * piece - 1] = 1;
        free(large);
    }
    return true;
}

int main(void)
{
    static const size_t sizes[] = {1, 15, 16, 17, 31, 32, 33, 100, 1000, 4096};
    unsigned char *ptr;
    unsigned char *other;
    unsigned char *big;
    void *empty;
    void *empty2;
    size_t i;
    int status;

    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        slot[i] = malloc(sizes[i]);
        if (slot[i] == NULL or not aligned(slot[i])) return 1;
        fill(slot[i], sizes[i], (unsigned char) i);
    }
    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        if (not filled(slot[i], sizes[i], (unsigned char) i)) return 2;
        free(slot[i]);
        slot[i] = NULL;
    }

    empty = malloc(0);
    empty2 = malloc(0);
    if (empty != NULL and empty == empty2) return 3;
    free(empty);
    free(empty2);
    free(NULL);

    ptr = malloc(256);
    if (ptr == NULL) return 4;
    memset(ptr, 0xff, 256);
    free(ptr);
    ptr = calloc(16, 16);
    if (ptr == NULL or not aligned(ptr)) return 4;
    for (i = 0; i < 256; i++) {
        if (ptr[i] != 0) return 4;
    }
    free(ptr);
    free(calloc(0, 16));

    if (calloc(huge / 2 + 2, 2) != NULL) return 5;
    if (malloc(huge) != NULL) return 5;

    ptr = realloc(NULL, 100);
    if (ptr == NULL or not aligned(ptr)) return 6;
    fill(ptr, 100, 3);
    if (realloc(ptr, huge) != NULL or not filled(ptr, 100, 3)) return 6;
    ptr = realloc(ptr, 5000);
    if (ptr == NULL or not aligned(ptr) or not filled(ptr, 100, 3)) return 6;
    fill(ptr, 5000, 4);
    ptr = realloc(ptr, 10);
    if (ptr == NULL or not filled(ptr, 10, 4)) return 6;
    free(ptr);

    ptr = malloc(100);
    other = malloc(100);
    if (ptr == NULL or other == NULL) return 7;
    fill(ptr, 100, 5);
    free(other);
    ptr = realloc(ptr, 180);
    if (ptr == NULL or not filled(ptr, 100, 5)) return 7;
    free(ptr);

    big = malloc(BIG);
    if (big == NULL or not aligned(big)) return 8;
    fill(big, BIG, 6);
    if (not filled(big, BIG, 6)) return 8;
    free(big);

    status = churn();
    if (status != 0) return 10 + status;

    if (not merges()) return 20;

    ptr = (malloc)(1);
    ptr = (realloc)(ptr, 2);
    if (ptr == NULL) return 21;
    (free)(ptr);
    ptr = (calloc)(1, 1);
    if (ptr == NULL or *ptr != 0) return 21;
    free(ptr);
    return 0;
}
