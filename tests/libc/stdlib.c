// (Test) Status: 0
// <stdlib.h> defines EXIT_FAILURE, EXIT_SUCCESS, RAND_MAX, at least 32767, and MB_CUR_MAX, never more than MB_LEN_MAX (S7.20p3), and declares the numeric conversions (S7.20.1),
// strtod, strtof and strtold reading each form of a number to the nearest value, an infinity, a NaN, and a number out of range as a huge or tiny one, and strtol and its kin a number of any base up to its type's limits,
// rand, which srand makes draw the same numbers again (S7.20.2), calloc, free, malloc and realloc, whose blocks are aligned for any type and keep their bytes while others come and go (S7.20.3),
// abort, which a handler of SIGABRT may leave, atexit and exit, which runs the functions registered last first, _Exit, which runs none, getenv and system (S7.20.4),
// bsearch and qsort (S7.20.5), abs, labs, llabs, div, ldiv and lldiv (S7.20.6), and mblen, mbtowc, wctomb, mbstowcs and wcstombs, in the "C" locale and in UTF-8 (S7.20.7-8),
// each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

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
#include <locale.h>
#include <stdlib.h>

// Whether a directive finds the macros defined, and RAND_MAX large enough.
#define DIRECTIVE 0
#if defined(EXIT_FAILURE) \
    && defined(EXIT_SUCCESS) \
    && defined(RAND_MAX) \
    && (RAND_MAX >= 32767)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

#define NONE      (-1)
#define SIZES     10
#define SLOTS     100
#define STEPS     1500
#define RARELY    4
#define LONGEST   2000
#define SHORTEST  100
#define SMALL     16
#define PIECE     65536
#define GROWTH    16
#define SCATTER   7
#define ROUNDS    200
#define ZEROED    256
#define BLOCK     100
#define GROWN     5000
#define SHRUNK    10
#define WIDER     180
#define BIG       (1 << 17)
#define EXITS     32
#define MIDDLES   (EXITS - 3)
#define EXITED    60
#define SORTED    2000
#define RANGE     1000
#define KEYS      7
#define STRIDE    7
#define DRAWS     50
#define TEXT      16
#define UTF8      "C.UTF-8"
#define DBL_LEAST (DBL_MIN / 4503599627370496.0)

// The strictest alignment of the basic types, which malloc's memory must meet.
struct Strictest {
    char st_tag;
    union {
        long double st_ld;
        long long st_ll;
        double st_dbl;
        void *st_ptr;
    } st_all;
};

// The sizes of the blocks that live together, either side of a power of two.
static const size_t sizes[SIZES] = {1, 15, 16, 17, 31, 32, 33, 100, 1000, 4096};

static unsigned char *slot[SLOTS];
static size_t length[SLOTS];
static unsigned long seed = 1;
static volatile size_t huge = SIZE_MAX;
static volatile double nought = 0.0;
static volatile sig_atomic_t aborted;
static jmp_buf env;
static int stage;
static int middles;
static int draws[DRAWS];
static int sorted[SORTED];
static bool seen[SORTED];
static unsigned char records[SORTED][3];

// Return the next number of a sequence that is the same in every run.
static unsigned long next(void)
{
    seed = seed * 6364136223846793005UL + 1442695040888963407UL;
    return seed >> 33;
}

// Return how far a pointer is past the strictest alignment.
static unsigned long misaligned(const void *ptr)
{
    return (uintptr_t) ptr % offsetof(struct Strictest, st_all);
}

// Write n bytes that count up from tag.
static void fill(unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        ptr[i] = (unsigned char) (tag + i);
    }
}

// Return the place of the first of n bytes not counting up from tag, or NONE.
static long spoiled(const unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (ptr[i] != (unsigned char) (tag + i)) {
            return (long) i;
        }
    }
    return NONE;
}

// Allocate a block of each size, and return the first size malloc fails.
static long allocate(void)
{
    size_t i;

    for (i = 0; i < SIZES; i++) {
        slot[i] = malloc(sizes[i]);
        if (slot[i] == NULL || misaligned(slot[i]) != 0) {
            return (long) sizes[i];
        }
        fill(slot[i], sizes[i], (unsigned char) i);
    }
    return NONE;
}

// Free each size's block, and return the first size whose bytes had changed.
static long release(void)
{
    size_t i;
    long changed = NONE;

    for (i = 0; i < SIZES; i++) {
        if (changed == NONE && spoiled(slot[i], sizes[i], (unsigned char) i) != NONE) {
            changed = (long) sizes[i];
        }
        free(slot[i]);
        slot[i] = NULL;
    }
    return changed;
}

// Allocate, resize and free at random, each live block keeping its own bytes.
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

// Free small blocks and take a large one only they merged can hold, 200 times.
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

// Tell whether strtod reads a string as a value, bit for bit, and how far.
static bool reads(const char *str, double value, long end)
{
    int lost = 0;
    char *stop = NULL;
    double got = strtod(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// Tell whether strtof reads a string as a value, bit for bit, and how far.
static bool reads_float(const char *str, float value, long end)
{
    int lost = 0;
    char *stop = NULL;
    float got = strtof(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// Tell whether strtold reads a string as a value, bit for bit, and how far.
static bool reads_long(const char *str, long double value, long end)
{
    int lost = 0;
    char *stop = NULL;
    long double got = strtold(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// Tell whether strtod reads all of a string as too large, of the sign given.
static bool overflows(const char *str, int sign)
{
    int lost = 0;
    int number;
    double got;
    char *stop = NULL;

    errno = 0;
    got = strtod(str, &stop);
    number = errno;
    lost += ! _TEST_TRUE(sign * got >= DBL_MAX);
    lost += ! _TEST_INT(stop - str, strlen(str));
    lost += ! _TEST_INT(number, ERANGE);
    return lost == 0;
}

// Tell whether strtod reads all of a string as too small, of the sign given.
static bool underflows(const char *str, int sign)
{
    int lost = 0;
    char *stop = NULL;
    double got = strtod(str, &stop);

    lost += ! _TEST_TRUE(sign * got >= 0 && sign * got <= DBL_MIN);
    lost += ! _TEST_INT(stop - str, strlen(str));
    return lost == 0;
}

// Tell whether strtol reads a string in a base as a value, and how far.
static bool converts(const char *str, int base, long value, long end)
{
    int lost = 0;
    char *stop = NULL;
    long got = strtol(str, &stop, base);

    lost += ! _TEST_INT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// Draw DRAWS numbers, and return the place of the first out of range, or NONE.
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

// Return the place of the first draw rand does not give again, or NONE.
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

// Order two ints.
static int compare_int(const void *left, const void *right)
{
    int x = *(const int *) left;
    int y = *(const int *) right;

    return (x > y) - (x < y);
}

// Order three-byte records by their first byte alone.
static int compare_first(const void *left, const void *right)
{
    return *(const unsigned char *) left - *(const unsigned char *) right;
}

// Return the first place in sorted whose int is below the one before, or NONE.
static int descent(void)
{
    int i;

    for (i = 1; i < SORTED; i++) {
        if (sorted[i - 1] > sorted[i]) {
            return i;
        }
    }
    return NONE;
}

// Return the first place whose record is out of order, torn or there twice.
static int misplaced(void)
{
    int i;
    int index;

    for (i = 0; i < SORTED; i++) {
        index = records[i][1] | records[i][2] << CHAR_BIT;
        if (i > 0 && records[i - 1][0] > records[i][0]) {
            return i;
        }
        if (index >= SORTED || seen[index]) {
            return i;
        }
        seen[index] = true;
    }
    return NONE;
}

// Return the first place whose int bsearch finds no equal of, or NONE.
static int unfound(void)
{
    int i;
    const int *found;

    for (i = 0; i < SORTED; i += STRIDE) {
        found = bsearch(&sorted[i], sorted, SORTED, sizeof(sorted[0]), compare_int);
        if (found == NULL || *found != sorted[i]) {
            return i;
        }
    }
    return NONE;
}

// Leave abort, as a handler of SIGABRT may (S7.20.4.1p2).
static void on_abort(int sig)
{
    aborted = sig == SIGABRT;
    longjmp(env, 1);
}

// Registered first, so run last, which finish never lets it be.
static void skipped(void)
{
    int reached = 1;

    _TEST_INT(reached, 0);
    _Exit(_Test_Status());
}

// Registered second: each later one has run, and _Exit runs no other.
static void finish(void)
{
    _TEST_INT(stage, 1);
    _TEST_INT(middles, MIDDLES);
    _Exit(_Test_Status());
}

// Registered between them, MIDDLES times, and counted once last has run.
static void middle(void)
{
    if (stage == 1) {
        middles++;
    }
}

// Registered last, so run first.
static void last(void)
{
    if (stage == 0 && middles == 0) {
        stage = 1;
    }
}

// Register middle MIDDLES times, and return the first time atexit refuses.
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

// The macros are defined, in a directive too, and within their limits.
static void Test_StdlibMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_TRUE(RAND_MAX >= 32767);
    _TEST_TRUE(EXIT_FAILURE != EXIT_SUCCESS);
    _TEST_TRUE(MB_CUR_MAX >= 1 && MB_CUR_MAX <= MB_LEN_MAX);
    _TEST_TRUE(NULL == (void *) 0);
}

// malloc gives aligned blocks that keep their bytes, and none too large.
static void Test_StdlibMalloc(void)
{
    void *empty;
    void *other;
    unsigned char *big;

    if (_TEST_INT(allocate(), NONE)) {
        _TEST_INT(release(), NONE);
    }

    empty = malloc(0);
    other = malloc(0);
    _TEST_TRUE(empty == NULL || empty != other);
    free(empty);
    free(other);
    free(NULL);

    big = malloc(BIG);
    if (_TEST_TRUE(big != NULL)) {
        _TEST_UINT(misaligned(big), 0);
        fill(big, BIG, 6);
        _TEST_INT(spoiled(big, BIG, 6), NONE);
        free(big);
    }
    _TEST_TRUE(malloc(huge) == NULL);
}

// calloc gives zeroed memory, of a block freed too, and none too large.
static void Test_StdlibCalloc(void)
{
    unsigned char *ptr = malloc(ZEROED);
    static const unsigned char zeros[ZEROED];

    if (_TEST_TRUE(ptr != NULL)) {
        memset(ptr, UCHAR_MAX, ZEROED);
        free(ptr);
    }
    ptr = calloc(SMALL, ZEROED / SMALL);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_UINT(misaligned(ptr), 0);
        _TEST_MEM(ptr, zeros, ZEROED);
        free(ptr);
    }
    free(calloc(0, SMALL));
    _TEST_TRUE(calloc(huge / 2 + 2, 2) == NULL);
}

// realloc keeps a block's bytes as it grows, shrinks, moves or fails.
static void Test_StdlibRealloc(void)
{
    unsigned char *ptr = realloc(NULL, BLOCK);
    unsigned char *other;

    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_UINT(misaligned(ptr), 0);
    fill(ptr, BLOCK, 3);
    _TEST_TRUE(realloc(ptr, huge) == NULL);
    _TEST_INT(spoiled(ptr, BLOCK, 3), NONE);
    ptr = realloc(ptr, GROWN);
    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_UINT(misaligned(ptr), 0);
    _TEST_INT(spoiled(ptr, BLOCK, 3), NONE);
    fill(ptr, GROWN, 4);
    ptr = realloc(ptr, SHRUNK);
    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_INT(spoiled(ptr, SHRUNK, 4), NONE);
    free(ptr);

    ptr = malloc(BLOCK);
    other = malloc(BLOCK);
    if (! _TEST_TRUE(ptr != NULL && other != NULL)) {
        return;
    }
    fill(ptr, BLOCK, 5);
    free(other);
    ptr = realloc(ptr, WIDER);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(spoiled(ptr, BLOCK, 5), NONE);
        free(ptr);
    }
}

// Blocks that come and go at random keep their bytes, and freed ones merge.
static void Test_StdlibHeap(void)
{
    _TEST_TRUE(churn());
    _TEST_TRUE(merges());
}

// strtod reads each form of a decimal and a hexadecimal number, and no other.
static void Test_StdlibStrtodForms(void)
{
    _TEST_TRUE(reads("1.5", 1.5, 3));
    _TEST_TRUE(reads("  -2.25e1", -22.5, 9));
    _TEST_TRUE(reads("+.5", 0.5, 3));
    _TEST_TRUE(reads("5.", 5.0, 2));
    _TEST_TRUE(reads("12abc", 12.0, 2));
    _TEST_TRUE(reads("\t\n\v\f\r 7", 7.0, 7));
    _TEST_TRUE(reads("1.5.5", 1.5, 3));
    _TEST_TRUE(reads("0x1.8.8", 1.5, 5));
    _TEST_TRUE(reads("1e", 1.0, 1));
    _TEST_TRUE(reads("1e+", 1.0, 1));
    _TEST_TRUE(reads("1e+x", 1.0, 1));
    _TEST_TRUE(reads("1.5E+00002", 150.0, 10));
    _TEST_TRUE(reads("0x", 0.0, 1));
    _TEST_TRUE(reads("0x1.8p1", 3.0, 7));
    _TEST_TRUE(reads("0X1P-2", 0.25, 6));
    _TEST_TRUE(reads("0x1p", 1.0, 3));
    _TEST_TRUE(reads("0x.8", 0.5, 4));
    _TEST_TRUE(reads("-0xAbp0", -171.0, 7));

    _TEST_TRUE(reads("", 0.0, 0));
    _TEST_TRUE(reads(".", 0.0, 0));
    _TEST_TRUE(reads("-", 0.0, 0));
    _TEST_TRUE(reads("e5", 0.0, 0));
    _TEST_TRUE(reads(".e1", 0.0, 0));
    _TEST_TRUE(reads("x", 0.0, 0));
}

// strtod reads an infinity and a NaN, in any case, with a NaN's characters.
static void Test_StdlibStrtodSpecials(void)
{
    double invalid = nought / nought;
    double infinite = 1 / nought;

    _TEST_TRUE(reads("inf", infinite, 3));
    _TEST_TRUE(reads("INFINITY", infinite, 8));
    _TEST_TRUE(reads("-Inf", -infinite, 4));
    _TEST_TRUE(reads("infinit", infinite, 3));

    _TEST_TRUE(reads("nan", invalid, 3));
    _TEST_TRUE(reads("-NaN", invalid, 4));
    _TEST_TRUE(reads("nan()", invalid, 5));
    _TEST_TRUE(reads("nan(123)", invalid, 8));
    _TEST_TRUE(reads("NAN(abc_9)", invalid, 10));
    _TEST_TRUE(reads("nan(", invalid, 3));
    _TEST_TRUE(reads("nan(-1)", invalid, 3));
}

// strtod reads a zero of either sign and the least and greatest values.
static void Test_StdlibStrtodLimits(void)
{
    _TEST_TRUE(reads("0", 0.0, 1));
    _TEST_TRUE(reads("-0", -nought, 2));
    _TEST_TRUE(reads("-0x0p3", -nought, 6));
    _TEST_TRUE(reads("0x1p-1022", DBL_MIN, 9));
    _TEST_TRUE(reads("0x1.fffffffffffffp1023", DBL_MAX, 22));
    _TEST_TRUE(reads("0x1p-1074", DBL_LEAST, 9));
    _TEST_TRUE(reads("0x0.0000000000001p-1022", DBL_LEAST, 23));
    _TEST_TRUE(reads("1e0000000000000000000000000000000001", 10.0, 36));
    _TEST_TRUE(reads("0e999999999999999999999", 0.0, 23));
}

// strtod rounds a number between two values to the nearest, a tie to even.
static void Test_StdlibStrtodRounding(void)
{
    _TEST_TRUE(reads("0.1", 0.1, 3));
    _TEST_TRUE(reads("2.2250738585072014e-308", DBL_MIN, 23));
    _TEST_TRUE(reads("1.7976931348623157e308", DBL_MAX, 22));
    _TEST_TRUE(reads("9007199254740993", 9007199254740992.0, 16));
    _TEST_TRUE(reads("9007199254740995", 9007199254740996.0, 16));
    _TEST_TRUE(reads("123456789012345678901", 123456789012345678901.0, 21));
    _TEST_TRUE(reads("2.4703282292062328e-324", DBL_LEAST, 23));

    _TEST_TRUE(reads("0x1.00000000000008p0", 1.0, 20));
    _TEST_TRUE(reads("0x1.00000000000018p0", 0x1.0000000000002p0, 20));
    _TEST_TRUE(reads("0x1.000000000000081p0", 0x1.0000000000001p0, 21));
    _TEST_TRUE(reads("0x1.fffffffffffff8p0", 2.0, 20));
}

// strtod reads a number out of range as a huge or a tiny one of its sign.
static void Test_StdlibStrtodRange(void)
{
    _TEST_TRUE(overflows("1e400", 1));
    _TEST_TRUE(overflows("-1e400", -1));
    _TEST_TRUE(overflows("0x1p1024", 1));
    _TEST_TRUE(underflows("1e-400", 1));
    _TEST_TRUE(underflows("-1e-400", -1));
    _TEST_TRUE(underflows("0x1p-1080", 1));
}

// strtof reads to the nearest float, and a number out of its range as strtod.
static void Test_StdlibStrtof(void)
{
    _TEST_TRUE(reads_float("1.5", 1.5F, 3));
    _TEST_TRUE(reads_float("3.4028235e38", FLT_MAX, 12));
    _TEST_TRUE(reads_float("0x1p-126", FLT_MIN, 8));
    _TEST_TRUE(reads_float("0x1.000001p0", 1.0F, 12));
    _TEST_TRUE(reads_float("0x1.000003p0", 0x1.000004p0F, 12));
    _TEST_TRUE(reads_float("0.1", 0.1F, 3));
    errno = 0;
    _TEST_TRUE(strtof("3.5e38", NULL) > FLT_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_TRUE(strtof("1e-50", NULL) <= FLT_MIN);
}

// strtold reads to the nearest long double, up to its limits and no further.
static void Test_StdlibStrtold(void)
{
    _TEST_TRUE(reads_long("1.5", 1.5L, 3));
    _TEST_TRUE(reads_long("0.1", 0.1L, 3));
    _TEST_TRUE(reads_long("-0x1p-16382", -LDBL_MIN, 11));
    _TEST_TRUE(reads_long("0x1.fffffffffffffffep16383", LDBL_MAX, 26));
    _TEST_TRUE(reads_long("3.36210314311209350626e-4932", LDBL_MIN, 28));
    errno = 0;
    _TEST_TRUE(strtold("1e5000", NULL) > LDBL_MAX);
    _TEST_INT(errno, ERANGE);
}

// strtol and its kin read a number of any base, and a limit past their range.
static void Test_StdlibIntegers(void)
{
    char *stop = NULL;
    const char *most = "99999999999999999999";

    _TEST_TRUE(converts("  -123abc", 10, -123, 6));
    _TEST_TRUE(converts("0x1F", 0, 31, 4));
    _TEST_TRUE(converts("017", 0, 15, 3));
    _TEST_TRUE(converts("z", 36, 35, 1));
    _TEST_TRUE(converts("101", 2, 5, 3));
    _TEST_TRUE(converts("-0x10", 16, -16, 5));
    _TEST_TRUE(converts("0x", 16, 0, 1));
    _TEST_TRUE(converts("09", 0, 0, 1));
    _TEST_TRUE(converts("  +", 10, 0, 0));
    _TEST_TRUE(converts("", 0, 0, 0));

    errno = 0;
    _TEST_INT(strtol(most, &stop, 10), LONG_MAX);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(stop - most, 20);
    errno = 0;
    _TEST_INT(strtoll("-99999999999999999999", NULL, 10), LLONG_MIN);
    _TEST_INT(errno, ERANGE);
    errno = 0;
    _TEST_INT(strtol("-99999999999999999999", NULL, 0), LONG_MIN);
    _TEST_INT(errno, ERANGE);
    _TEST_INT(strtoll("12", NULL, 0), 12);

    _TEST_UINT(strtoul("-1", NULL, 10), ULONG_MAX);
    _TEST_UINT(strtoull("-1", NULL, 10), ULLONG_MAX);
    _TEST_UINT(strtoul("0xffff", NULL, 16), 0xffff);
    errno = 0;
    _TEST_UINT(strtoull("999999999999999999999999", NULL, 10), ULLONG_MAX);
    _TEST_INT(errno, ERANGE);

    _TEST_INT(atoi("  42x"), 42);
    _TEST_INT(atol("-7"), -7);
    _TEST_INT(atoll("123456789012"), 123456789012LL);
    _TEST_FLOAT(atof("2.5x"), 2.5);
}

// rand draws within its range, and the same numbers again after their seed.
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

// qsort orders its elements by the comparison alone, each kept whole.
static void Test_StdlibSort(void)
{
    int i;
    long sum = 0;
    long squares = 0;

    for (i = 0; i < SORTED; i++) {
        sorted[i] = (int) (next() % RANGE) - RANGE / 2;
        sum += sorted[i];
        squares += (long) sorted[i] * sorted[i];
    }
    qsort(sorted, SORTED, sizeof(sorted[0]), compare_int);
    _TEST_INT(descent(), NONE);
    for (i = 0; i < SORTED; i++) {
        sum -= sorted[i];
        squares -= (long) sorted[i] * sorted[i];
    }
    _TEST_INT(sum, 0);
    _TEST_INT(squares, 0);

    for (i = 0; i < SORTED; i++) {
        records[i][0] = (unsigned char) (next() % KEYS);
        records[i][1] = (unsigned char) (i & UCHAR_MAX);
        records[i][2] = (unsigned char) (i >> CHAR_BIT);
    }
    (qsort)(records, SORTED, sizeof(records[0]), compare_first);
    _TEST_INT(misplaced(), NONE);

    qsort(sorted, 0, sizeof(sorted[0]), compare_int);
    qsort(sorted, 1, sizeof(sorted[0]), compare_int);
    _TEST_INT(descent(), NONE);
}

// bsearch finds an element of the ints qsort left in order, and no other.
static void Test_StdlibSearch(void)
{
    int missing = RANGE;

    _TEST_INT(unfound(), NONE);
    _TEST_TRUE(bsearch(&missing, sorted, SORTED, sizeof(sorted[0]), compare_int) == NULL);
    _TEST_TRUE((bsearch)(&sorted[0], sorted, 0, sizeof(sorted[0]), compare_int) == NULL);
}

// The absolute values, and quotients truncated toward zero with remainders.
static void Test_StdlibArithmetic(void)
{
    _TEST_INT(abs(-5), 5);
    _TEST_INT(abs(INT_MIN + 1), INT_MAX);
    _TEST_INT(labs(-5L), 5);
    _TEST_INT(labs(LONG_MIN + 1), LONG_MAX);
    _TEST_INT(llabs(-5LL), 5);
    _TEST_INT(llabs(LLONG_MIN + 1), LLONG_MAX);

    _TEST_INT(div(-7, 2).quot, -3);
    _TEST_INT(div(-7, 2).rem, -1);
    _TEST_INT(ldiv(7L, -2L).quot, -3);
    _TEST_INT(ldiv(7L, -2L).rem, 1);
    _TEST_INT(lldiv(-7LL, -2LL).quot, 3);
    _TEST_INT(lldiv(-7LL, -2LL).rem, -1);
}

// In the "C" locale a multibyte character is one byte, with no shift states.
static void Test_StdlibMultibyte(void)
{
    wchar_t wc = L'?';
    char bytes[TEXT];
    wchar_t wide[TEXT];

    _TEST_INT(mbtowc(&wc, "A", 1), 1);
    _TEST_INT(wc, L'A');
    _TEST_INT(mbtowc(&wc, "", 1), 0);
    _TEST_INT(wc, L'\0');
    _TEST_INT(mbtowc(NULL, "A", 1), 1);
    _TEST_INT(mblen("A", 1), 1);
    _TEST_INT(mblen("", 1), 0);
    _TEST_INT(mblen("A", 0), -1);
    _TEST_INT((mblen)("AB", 2), 1);
    _TEST_INT((mbtowc)(&wc, "B", 1), 1);
    _TEST_INT(wc, L'B');
    _TEST_INT(mbtowc(NULL, NULL, 0) != 0, wctomb(NULL, L'\0') != 0);
    _TEST_INT(mblen(NULL, 0) != 0, wctomb(NULL, L'\0') != 0);
    _TEST_INT(wctomb(bytes, L'A'), 1);
    _TEST_INT(bytes[0], 'A');
    _TEST_INT((wctomb)(bytes, L'\0'), 1);
    _TEST_INT(bytes[0], '\0');

    _TEST_UINT(mbstowcs(wide, "hello", 8), 5);
    _TEST_MEM(wide, L"hello", 6 * sizeof(wchar_t));
    wide[3] = L'x';
    _TEST_UINT((mbstowcs)(wide, "abcde", 3), 3);
    _TEST_MEM(wide, L"abcx", 4 * sizeof(wchar_t));
    _TEST_UINT(wcstombs(bytes, L"hi", sizeof(bytes)), 2);
    _TEST_STR(bytes, "hi");
    bytes[3] = 'x';
    _TEST_UINT((wcstombs)(bytes, L"hello", 3), 3);
    _TEST_MEM(bytes, "helx", 4);
}

// In a UTF-8 locale a character is one to four bytes, and a stray byte none.
static void Test_StdlibUtf8(void)
{
    wchar_t wc = L'?';
    char bytes[TEXT];
    wchar_t wide[TEXT];

    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(MB_CUR_MAX >= 1 && MB_CUR_MAX <= MB_LEN_MAX);
    _TEST_INT(mbtowc(&wc, "\xc3\xa9", 2), 2);
    _TEST_INT(wc, 0xe9);
    _TEST_INT(mblen("\xe2\x82\xac", 3), 3);
    _TEST_INT(mblen("\xc3\xa9", 1), -1);
    _TEST_INT(mbtowc(&wc, "\xff", 1), -1);
    _TEST_INT(mbtowc(&wc, "\xc3" "A", 2), -1);
    _TEST_INT(wctomb(bytes, 0x20ac), 3);
    _TEST_MEM(bytes, "\xe2\x82\xac", 3);
    _TEST_UINT(mbstowcs(wide, "a\xc3\xa9\xe2\x82\xac", 8), 3);
    _TEST_INT(wide[0], L'a');
    _TEST_INT(wide[1], 0xe9);
    _TEST_INT(wide[2], 0x20ac);
    _TEST_INT(wide[3], L'\0');
    _TEST_UINT(wcstombs(bytes, wide, sizeof(bytes)), 6);
    _TEST_STR(bytes, "a\xc3\xa9\xe2\x82\xac");
    _TEST_UINT(wcstombs(bytes, wide, 2), 1);
    _TEST_UINT(mbstowcs(wide, "a\xff", 8), (size_t) -1);
    _TEST_UINT(wcstombs(bytes, L"", 1), 0);
    _TEST_INT(bytes[0], '\0');
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

// getenv gives a variable's value, the same each time, and none for no other.
static void Test_StdlibEnvironment(void)
{
    const char *value = getenv("IVANCC_TEST");

    _TEST_STR(value, "1");
    _TEST_TRUE((getenv)("IVANCC_TEST") == value);
    _TEST_STR(getenv("IVANCC"), NULL);
    _TEST_STR(getenv("IVANCC_TEST_UNSET"), NULL);
    (void) system(NULL);
    (void) (system)(NULL);
}

// A pointer reaches each function, and a call in parentheses any macro's.
static void Test_StdlibCalls(void)
{
    unsigned char *ptr = (malloc)(1);
    void *(*zeroed)(size_t, size_t) = calloc;
    long (*convert)(const char *restrict, char **restrict, int) = strtol;

    ptr = (realloc)(ptr, 2);
    _TEST_TRUE(ptr != NULL);
    (free)(ptr);
    ptr = (calloc)(1, 1);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(*ptr, 0);
        free(ptr);
    }
    ptr = zeroed(1, 1);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(*ptr, 0);
        free(ptr);
    }
    _TEST_INT(convert("-12", NULL, 10), -12);

    _TEST_FLOAT((atof)("-1"), -1.0);
    _TEST_INT((atoi)("3"), 3);
    _TEST_INT((atol)("3"), 3);
    _TEST_INT((atoll)("3"), 3);
    _TEST_FLOAT((strtod)("4", NULL), 4.0);
    _TEST_FLOAT((strtof)("4", NULL), 4.0F);
    _TEST_FLOAT((strtold)("4", NULL), 4.0L);
    _TEST_INT((strtol)("7", NULL, 10), 7);
    _TEST_INT((strtoll)("7", NULL, 10), 7);
    _TEST_UINT((strtoul)("7", NULL, 10), 7);
    _TEST_UINT((strtoull)("7", NULL, 10), 7);
    _TEST_INT((abs)(5), 5);
    _TEST_INT((labs)(-5L), 5);
    _TEST_INT((llabs)(-5LL), 5);
    _TEST_INT((div)(7, 2).quot, 3);
    _TEST_INT((ldiv)(7L, 2L).rem, 1);
    _TEST_INT((lldiv)(7LL, 2LL).quot, 3);
}

// abort raises SIGABRT, whose handler may leave it by longjmp.
static void Test_StdlibAbort(void)
{
    volatile int returned = 0;

    aborted = 0;
    if (_TEST_TRUE(signal(SIGABRT, on_abort) != SIG_ERR)) {
        if (setjmp(env) == 0) {
            abort();
            returned = 1;
        }
        _TEST_INT(aborted, 1);
        _TEST_INT(returned, 0);
        _TEST_TRUE(signal(SIGABRT, SIG_DFL) != SIG_ERR);
    }
}

// exit runs the functions registered, last first, and finish ends the program.
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
    Test_StdlibMacros();
    Test_StdlibMalloc();
    Test_StdlibCalloc();
    Test_StdlibRealloc();
    Test_StdlibHeap();
    Test_StdlibStrtodForms();
    Test_StdlibStrtodSpecials();
    Test_StdlibStrtodLimits();
    Test_StdlibStrtodRounding();
    Test_StdlibStrtodRange();
    Test_StdlibStrtof();
    Test_StdlibStrtold();
    Test_StdlibIntegers();
    Test_StdlibRand();
    Test_StdlibSort();
    Test_StdlibSearch();
    Test_StdlibArithmetic();
    Test_StdlibMultibyte();
    Test_StdlibUtf8();
    Test_StdlibEnvironment();
    Test_StdlibCalls();
    Test_StdlibAbort();
    Test_StdlibExit();
    return _Test_Status();
}
