// (Test) Status: 0
// <stdlib.h> declares the numeric conversion functions, atof, atoi, atol, atoll, strtod, strtof, strtold,
// strtol, strtoll, strtoul and strtoull (S7.20.1), rand and srand (S7.20.2),
// the memory management functions, calloc, free, malloc and realloc (S7.20.3),
// those of communication with the environment, abort, atexit, exit, _Exit, getenv and system (S7.20.4),
// bsearch and qsort (S7.20.5), abs, labs, llabs, div, ldiv and lldiv (S7.20.6),
// mblen, mbtowc and wctomb (S7.20.7), and mbstowcs and wcstombs (S7.20.8).

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
#define EXITS 32
#define EXITED 60
#define SORTED 2000
#define DRAWS 50

#ifndef EXIT_FAILURE
#error "EXIT_FAILURE is not defined"
#endif

#ifndef EXIT_SUCCESS
#error "EXIT_SUCCESS is not defined"
#endif

#if ! defined(RAND_MAX) || RAND_MAX < 32767
#error "RAND_MAX is not defined as at least 32767"
#endif

static unsigned char *slot[SLOTS];
static size_t length[SLOTS];
static unsigned long seed = 1;
static volatile size_t huge = SIZE_MAX;
static jmp_buf env;
static volatile sig_atomic_t aborted;
static int stage;
static int middles;
static int sorted[SORTED];
static unsigned char records[SORTED][3];
static bool seen[SORTED];
static int draws[DRAWS];

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

// Check that strtod reads str as value, bit for bit, ending end characters in.
static bool reads(const char *str, double value, size_t end)
{
    char *stop;
    double got = strtod(str, &stop);

    return memcmp(&got, &value, sizeof(got)) == 0 and stop == str + end;
}

// Check that strtof reads str as value, bit for bit, ending end characters in.
static bool reads_float(const char *str, float value, size_t end)
{
    char *stop;
    float got = strtof(str, &stop);

    return memcmp(&got, &value, sizeof(got)) == 0 and stop == str + end;
}

// Check that strtold reads str as value, ending end characters in.
static bool reads_long(const char *str, long double value, size_t end)
{
    char *stop;
    long double got = strtold(str, &stop);

    return got == value and stop == str + end;
}

// Check that strtod reads no number from str, giving 0 and str itself.
static bool refuses(const char *str)
{
    char *stop;

    return strtod(str, &stop) == 0 and stop == str;
}

// Check that strtod reads str as a NaN, ending end characters in.
static bool reads_nan(const char *str, size_t end)
{
    char *stop;
    double got = strtod(str, &stop);

    return got != got and stop == str + end;
}

// Check that strtod reads all of str as an infinity or HUGE_VAL of sign.
static bool reads_huge(const char *str, int sign, bool range)
{
    char *stop;
    double got;

    errno = 0;
    got = strtod(str, &stop);
    if (stop != str + strlen(str) or (range and errno != ERANGE)) return false;
    return sign > 0 ? got >= DBL_MAX : got <= -DBL_MAX;
}

// Check that strtod reads all of str as an underflow, of magnitude at most DBL_MIN (S7.20.1.3p10).
static bool reads_tiny(const char *str, int sign)
{
    char *stop;
    double got = strtod(str, &stop);

    if (stop != str + strlen(str)) return false;
    return sign > 0 ? got >= 0 and got <= DBL_MIN : got <= 0 and got >= -DBL_MIN;
}

// Check that strtol reads str in base as value, ending end characters in.
static bool converts(const char *str, int base, long value, size_t end)
{
    char *stop;

    return strtol(str, &stop, base) == value and stop == str + end;
}

// Check that rand gives the draws again, as after the seed that made them.
static bool repeats(void)
{
    int i;

    for (i = 0; i < DRAWS; i++) {
        if (rand() != draws[i]) return false;
    }
    return true;
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

// Sort SORTED ints and check they come out in order, the same ints as went in.
static bool sorts_ints(void)
{
    long sum = 0;
    long squares = 0;
    int i;

    for (i = 0; i < SORTED; i++) {
        sorted[i] = (int) (next() % 1000) - 500;
        sum += sorted[i];
        squares += (long) sorted[i] * sorted[i];
    }
    qsort(sorted, SORTED, sizeof(sorted[0]), compare_int);
    for (i = 0; i < SORTED; i++) {
        if (i > 0 and sorted[i - 1] > sorted[i]) return false;
        sum -= sorted[i];
        squares -= (long) sorted[i] * sorted[i];
    }
    return sum == 0 and squares == 0;
}

// Sort records of three bytes by a key of few values, each record kept whole.
static bool sorts_records(void)
{
    int i;

    for (i = 0; i < SORTED; i++) {
        records[i][0] = (unsigned char) (next() % 7);
        records[i][1] = (unsigned char) (i & 0xff);
        records[i][2] = (unsigned char) (i >> 8);
    }
    (qsort)(records, SORTED, sizeof(records[0]), compare_first);
    for (i = 0; i < SORTED; i++) {
        int index = records[i][1] | records[i][2] << 8;

        if (i > 0 and records[i - 1][0] > records[i][0]) return false;
        if (index >= SORTED or seen[index]) return false;
        seen[index] = true;
    }
    return true;
}

// Leave abort, as (S7.20.4.1p2) lets a handler of SIGABRT do.
static void on_abort(int sig)
{
    aborted = sig == SIGABRT;
    longjmp(env, 1);
}

// Registered first, so run last: _Exit ends the program before it.
static void skipped(void)
{
    _Exit(EXITED + 2);
}

// Registered second: every later one has run, newest first.
static void finish(void)
{
    _Exit(stage == 1 and middles == EXITS - 3 ? 0 : EXITED + 1);
}

// Registered between them, 29 times.
static void middle(void)
{
    if (stage == 1) middles++;
}

// Registered last, so run first.
static void last(void)
{
    if (stage == 0 and middles == 0) stage = 1;
}

int main(void)
{
    static const size_t sizes[] = {1, 15, 16, 17, 31, 32, 33, 100, 1000, 4096};
    unsigned char *ptr;
    unsigned char *other;
    unsigned char *big;
    void *empty;
    void *empty2;
    char *value;
    char *stop;
    double zero = 0.0;
    size_t i;
    int status;
    int missing = 1000;
    wchar_t wc;
    wchar_t wide[16];
    char bytes[16];

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

    if (not reads("1.5", 1.5, 3) or not reads("  -2.25e1", -22.5, 9) or not reads("+.5", 0.5, 3)) return 22;
    if (not reads("5.", 5.0, 2) or not reads("12abc", 12.0, 2) or not reads("\t\n\v\f\r 7", 7.0, 7)) return 22;
    if (not reads("1.5.5", 1.5, 3) or not reads("0x1.8.8", 1.5, 5)) return 22;
    if (not reads("1e", 1.0, 1) or not reads("1e+", 1.0, 1) or not reads("1e+x", 1.0, 1) or not reads("1.5E+00002", 150.0, 10)) return 22;
    if (not reads("0x", 0.0, 1) or not reads("0x1.8p1", 3.0, 7) or not reads("0X1P-2", 0.25, 6)) return 22;
    if (not reads("0x1p", 1.0, 3) or not reads("0x.8", 0.5, 4) or not reads("-0xAbp0", -171.0, 7)) return 22;
    if (not refuses("") or not refuses(".") or not refuses("-") or not refuses("e5") or not refuses(".e1") or not refuses("x")) return 22;

    if (not reads_huge("inf", 1, false) or not reads_huge("INFINITY", 1, false) or not reads_huge("-Inf", -1, false)) return 23;
    value = "infinit";
    if (strtod(value, &stop) <= DBL_MAX or stop != value + 3) return 23;
    if (not reads_nan("nan", 3) or not reads_nan("-NaN", 4) or not reads_nan("nan()", 5)) return 23;
    if (not reads_nan("nan(123)", 8) or not reads_nan("NAN(abc_9)", 10) or not reads_nan("nan(", 3) or not reads_nan("nan(-1)", 3)) return 23;

    if (not reads("0", 0.0, 1) or not reads("-0", -zero, 2) or not reads("-0x0p3", -zero, 6)) return 24;
    if (not reads("0x1p-1022", DBL_MIN, 9) or not reads("0x1.fffffffffffffp1023", DBL_MAX, 22)) return 24;
    if (not reads("0x1p-1074", DBL_MIN / 4503599627370496.0, 9) or not reads("0x0.0000000000001p-1022", DBL_MIN / 4503599627370496.0, 23)) return 24;
    if (not reads("1e0000000000000000000000000000000001", 10.0, 36) or not reads("0e999999999999999999999", 0.0, 23)) return 24;

    if (not reads("0.1", 0.1, 3) or not reads("2.2250738585072014e-308", DBL_MIN, 23) or not reads("1.7976931348623157e308", DBL_MAX, 22)) return 25;
    if (not reads("9007199254740993", 9007199254740992.0, 16) or not reads("9007199254740995", 9007199254740996.0, 16)) return 25;
    if (not reads("123456789012345678901", 123456789012345678901.0, 21) or not reads("2.4703282292062328e-324", DBL_MIN / 4503599627370496.0, 23)) return 25;

    if (not reads("0x1.00000000000008p0", 1.0, 20) or not reads("0x1.00000000000018p0", 0x1.0000000000002p0, 20)) return 26;
    if (not reads("0x1.000000000000081p0", 0x1.0000000000001p0, 21) or not reads("0x1.fffffffffffff8p0", 2.0, 20)) return 26;

    if (not reads_huge("1e400", 1, true) or not reads_huge("-1e400", -1, true) or not reads_huge("0x1p1024", 1, true)) return 27;
    if (not reads_tiny("1e-400", 1) or not reads_tiny("-1e-400", -1) or not reads_tiny("0x1p-1080", 1)) return 27;

    if (not reads_float("1.5", 1.5f, 3) or not reads_float("3.4028235e38", FLT_MAX, 12) or not reads_float("0x1p-126", FLT_MIN, 8)) return 28;
    if (not reads_float("0x1.000001p0", 1.0f, 12) or not reads_float("0x1.000003p0", 0x1.000004p0f, 12) or not reads_float("0.1", 0.1f, 3)) return 28;
    errno = 0;
    if (strtof("3.5e38", NULL) <= FLT_MAX or errno != ERANGE or strtof("1e-50", NULL) > FLT_MIN) return 28;

    if (not reads_long("1.5", 1.5L, 3) or not reads_long("0.1", 0.1L, 3) or not reads_long("-0x1p-16382", -LDBL_MIN, 11)) return 29;
    if (not reads_long("0x1.fffffffffffffffep16383", LDBL_MAX, 26) or not reads_long("3.36210314311209350626e-4932", LDBL_MIN, 28)) return 29;
    errno = 0;
    if (strtold("1e5000", NULL) <= LDBL_MAX or errno != ERANGE) return 29;
    if (atof("2.5x") != 2.5 or (atof)("-1") != -1.0 or (strtod)("4", NULL) != 4.0 or (strtof)("4", NULL) != 4.0f or (strtold)("4", NULL) != 4.0L) return 29;

    if (not converts("  -123abc", 10, -123, 6) or not converts("0x1F", 0, 31, 4) or not converts("017", 0, 15, 3)) return 30;
    if (not converts("z", 36, 35, 1) or not converts("101", 2, 5, 3) or not converts("-0x10", 16, -16, 5)) return 30;
    if (not converts("0x", 16, 0, 1) or not converts("09", 0, 0, 1) or not converts("  +", 10, 0, 0) or not converts("", 0, 0, 0)) return 30;

    value = "99999999999999999999";
    errno = 0;
    if (strtol(value, &stop, 10) != LONG_MAX or errno != ERANGE or stop != value + 20) return 31;
    errno = 0;
    if (strtoll("-99999999999999999999", NULL, 10) != LLONG_MIN or errno != ERANGE) return 31;
    errno = 0;
    if (strtol("-99999999999999999999", NULL, 0) != LONG_MIN or errno != ERANGE or strtoll("12", NULL, 0) != 12) return 31;

    if (strtoul("-1", NULL, 10) != ULONG_MAX or strtoull("-1", NULL, 10) != ULLONG_MAX or strtoul("0xffff", NULL, 16) != 0xffff) return 32;
    errno = 0;
    if (strtoull("999999999999999999999999", NULL, 10) != ULLONG_MAX or errno != ERANGE) return 32;
    if ((strtol)("7", NULL, 10) != 7 or (strtoll)("7", NULL, 10) != 7 or (strtoul)("7", NULL, 10) != 7 or (strtoull)("7", NULL, 10) != 7) return 32;

    if (atoi("  42x") != 42 or atol("-7") != -7 or atoll("123456789012") != 123456789012LL) return 33;
    if ((atoi)("3") != 3 or (atol)("3") != 3 or (atoll)("3") != 3) return 33;

    for (i = 0; i < DRAWS; i++) {
        draws[i] = rand();
        if (draws[i] < 0 or draws[i] > RAND_MAX) return 34;
    }
    srand(1);
    if (not repeats()) return 34;
    (srand)(5);
    for (i = 0; i < DRAWS; i++) {
        draws[i] = (rand)();
    }
    srand(5);
    if (not repeats()) return 34;

    if (not sorts_ints()) return 35;
    if (not sorts_records()) return 36;
    qsort(sorted, 0, sizeof(sorted[0]), compare_int);
    qsort(sorted, 1, sizeof(sorted[0]), compare_int);

    for (i = 0; i < SORTED; i += 7) {
        int *found = bsearch(&sorted[i], sorted, SORTED, sizeof(sorted[0]), compare_int);
        if (found == NULL or *found != sorted[i]) return 37;
    }
    if (bsearch(&missing, sorted, SORTED, sizeof(sorted[0]), compare_int) != NULL) return 37;
    if ((bsearch)(&sorted[0], sorted, 0, sizeof(sorted[0]), compare_int) != NULL) return 37;

    if (abs(-5) != 5 or abs(INT_MIN + 1) != INT_MAX or labs(-5L) != 5 or llabs(-5LL) != 5 or (abs)(5) != 5) return 38;
    if (div(-7, 2).quot != -3 or div(-7, 2).rem != -1 or ldiv(7L, -2L).quot != -3 or ldiv(7L, -2L).rem != 1) return 38;
    if (lldiv(-7LL, -2LL).quot != 3 or lldiv(-7LL, -2LL).rem != -1 or (div)(7, 2).quot != 3 or (ldiv)(7L, 2L).rem != 1) return 38;

    if (MB_CUR_MAX < 1 or MB_CUR_MAX > MB_LEN_MAX) return 39;
    if (mbtowc(&wc, "A", 1) != 1 or wc != L'A' or mbtowc(&wc, "", 1) != 0 or wc != L'\0' or mbtowc(NULL, "A", 1) != 1) return 39;
    if (mblen("A", 1) != 1 or mblen("", 1) != 0 or mblen("A", 0) != -1 or (mblen)("AB", 2) != 1 or (mbtowc)(&wc, "B", 1) != 1) return 39;
    if ((mbtowc(NULL, NULL, 0) != 0) != (wctomb(NULL, L'\0') != 0) or (mblen(NULL, 0) != 0) != (wctomb(NULL, L'\0') != 0)) return 39;
    if (wctomb(bytes, L'A') != 1 or bytes[0] != 'A' or (wctomb)(bytes, L'\0') != 1 or bytes[0] != '\0') return 39;

    if (mbstowcs(wide, "hello", 8) != 5 or wide[0] != L'h' or wide[4] != L'o' or wide[5] != L'\0') return 40;
    wide[3] = L'x';
    if ((mbstowcs)(wide, "abcde", 3) != 3 or wide[2] != L'c' or wide[3] != L'x') return 40;
    if (wcstombs(bytes, L"hi", sizeof(bytes)) != 2 or strcmp(bytes, "hi") != 0) return 40;
    bytes[3] = 'x';
    if ((wcstombs)(bytes, L"hello", 3) != 3 or memcmp(bytes, "helx", 4) != 0) return 40;

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL or MB_CUR_MAX < 1 or MB_CUR_MAX > MB_LEN_MAX) return 41;
    if (mbtowc(&wc, "\xc3\xa9", 2) != 2 or wc != 0xe9 or mblen("\xe2\x82\xac", 3) != 3) return 41;
    if (mblen("\xc3\xa9", 1) != -1 or mbtowc(&wc, "\xff", 1) != -1 or mbtowc(&wc, "\xc3" "A", 2) != -1) return 41;
    if (wctomb(bytes, 0x20ac) != 3 or memcmp(bytes, "\xe2\x82\xac", 3) != 0) return 41;
    if (mbstowcs(wide, "a\xc3\xa9\xe2\x82\xac", 8) != 3 or wide[1] != 0xe9 or wide[2] != 0x20ac or wide[3] != L'\0') return 41;
    if (wcstombs(bytes, wide, sizeof(bytes)) != 6 or strcmp(bytes, "a\xc3\xa9\xe2\x82\xac") != 0 or wcstombs(bytes, wide, 2) != 1) return 41;
    if (mbstowcs(wide, "a\xff", 8) != (size_t) -1 or wcstombs(bytes, L"", 1) != 0 or bytes[0] != '\0') return 41;
    if (setlocale(LC_CTYPE, "C") == NULL) return 41;

    value = getenv("IVANCC_TEST");
    if (value == NULL or strcmp(value, "1") != 0) return 50;
    if ((getenv)("IVANCC_TEST") != value) return 50;
    if (getenv("IVANCC") != NULL or getenv("IVANCC_TEST_UNSET") != NULL) return 50;
    (void) system(NULL);
    (void) (system)(NULL);

    if (signal(SIGABRT, on_abort) == SIG_ERR) return 51;
    if (setjmp(env) == 0) {
        abort();
        return 51;
    }
    if (not aborted) return 51;
    signal(SIGABRT, SIG_DFL);

    if (atexit(skipped) != 0 or atexit(finish) != 0) return 52;
    for (i = 0; i < EXITS - 3; i++) {
        if (atexit(middle) != 0) return 52;
    }
    if ((atexit)(last) != 0) return 52;
    (exit)(EXITED);
    return 53;
}
