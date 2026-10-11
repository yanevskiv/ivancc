// (Test) Status: 0
// <stddef.h> defines ptrdiff_t, the signed type of what subtracting two pointers yields, size_t, the unsigned type of what sizeof yields, and wchar_t, the type of a wide character constant (S7.17p2),
// NULL, a null pointer constant, and offsetof, an integer constant of type size_t that is a member's offset in bytes from the start of its structure (S7.17p3),
// that of a member of a member and of an array's element too, and of a union's member 0.

#include <_test.h>
#include <stddef.h>

#define TAIL  3
#define AHEAD 2

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

union Either {
    char ei_ch;
    long ei_number;
};

// Return how far a member lies into its structure, in bytes.
static long distance(const void *whole, const void *member)
{
    return (const char *) member - (const char *) whole;
}

// ptrdiff_t is signed and holds what subtracting two pointers yields.
static void Test_StddefPtrdiff(void)
{
    int numbers[TAIL];
    int *start = &numbers[0];
    int *ahead = &numbers[AHEAD];
    ptrdiff_t back = start - ahead;
    ptrdiff_t forth = ahead - start;

    _TEST_TRUE((ptrdiff_t) -1 < 0);
    _TEST_UINT(sizeof(ptrdiff_t), sizeof(ahead - start));
    _TEST_INT(forth, AHEAD);
    _TEST_INT(back, -AHEAD);
}

// size_t is unsigned and holds what sizeof yields.
static void Test_StddefSize(void)
{
    size_t size = sizeof(struct Pair);

    _TEST_TRUE((size_t) -1 > 0);
    _TEST_UINT(sizeof(size_t), sizeof(sizeof(int)));
    _TEST_UINT(size, sizeof(struct Pair));
}

// wchar_t is the type of a wide character constant and holds its value.
static void Test_StddefWchar(void)
{
    wchar_t null = L'\0';
    wchar_t wide = L'a';

    _TEST_UINT(sizeof(wchar_t), sizeof(L'a'));
    _TEST_INT(wide, 'a');
    _TEST_INT(null, 0);
}

// NULL is a null pointer constant: it compares equal to 0 and to no object.
static void Test_StddefNull(void)
{
    int number = 0;
    int *some = &number;
    char *none = NULL;
    void (*func)(void) = NULL;

    _TEST_TRUE(NULL == 0);
    _TEST_TRUE((void *) NULL == (void *) 0);
    _TEST_TRUE(none == NULL);
    _TEST_TRUE(func == NULL);
    _TEST_TRUE(! none);
    _TEST_TRUE(! func);
    _TEST_TRUE(some != NULL);
    some = NULL;
    _TEST_TRUE(some == none);
}

// offsetof is a member's offset in bytes, an integer constant of type size_t.
static void Test_StddefOffsetof(void)
{
    char sized[offsetof(struct Pair, pa_tail)];
    struct Pair pair;

    _TEST_UINT(offsetof(struct Pair, pa_ch), 0);
    _TEST_UINT(offsetof(struct Pair, pa_number), 4);
    _TEST_UINT(offsetof(struct Pair, pa_tail), 8);
    _TEST_UINT(offsetof(struct Pair, pa_tail[1]), 9);
    _TEST_UINT(offsetof(struct Pair, pa_inner), 16);
    _TEST_UINT(offsetof(struct Pair, pa_inner.in_second), 24);
    _TEST_UINT(offsetof(struct Inner, in_second), 8);
    _TEST_UINT(offsetof(union Either, ei_ch), 0);
    _TEST_UINT(offsetof(union Either, ei_number), 0);

    _TEST_UINT(sizeof(offsetof(struct Pair, pa_number)), sizeof(size_t));
    _TEST_TRUE(offsetof(struct Pair, pa_ch) - 1 > 0);
    _TEST_UINT(sizeof(sized), 8);

    _TEST_INT(distance(&pair, &pair.pa_ch), offsetof(struct Pair, pa_ch));
    _TEST_INT(distance(&pair, &pair.pa_number), offsetof(struct Pair, pa_number));
    _TEST_INT(distance(&pair, &pair.pa_tail), offsetof(struct Pair, pa_tail));
    _TEST_INT(distance(&pair, &pair.pa_tail[1]), offsetof(struct Pair, pa_tail[1]));
    _TEST_INT(distance(&pair, &pair.pa_inner), offsetof(struct Pair, pa_inner));
    _TEST_INT(distance(&pair, &pair.pa_inner.in_second), offsetof(struct Pair, pa_inner.in_second));
}

int main(void)
{
    Test_StddefPtrdiff();
    Test_StddefSize();
    Test_StddefWchar();
    Test_StddefNull();
    Test_StddefOffsetof();
    return _Test_Status();
}
