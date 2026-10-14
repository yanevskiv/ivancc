// (Test) Status: 42
// [C11:6.7.5] _Alignas, C11's, as gcc takes it under C99. It takes a type or
// a constant, on an object or a member. The strictest of several wins and a
// zero has no effect [C11:6.7.5p6]. Above 16 goes on static storage only.

enum {
    WORD  = 8,
    WIDE  = 16,
    CACHE = 64,
    PAGE  = 4096
};

struct Padded {
    char                mi_first;
    _Alignas(WORD) char mi_word;
    _Alignas(long double) char mi_wide;
    char                mi_last;
    _Alignas(0) char    mi_plain;
    _Alignas(WORD) char mi_one, mi_two;
};

union Either {
    char               ei_byte;
    _Alignas(int) char ei_bytes[3];
};

_Static_assert(__builtin_offsetof(struct Padded, mi_word) == 8, "on 8");
_Static_assert(__builtin_offsetof(struct Padded, mi_wide) == 16, "on 16");
_Static_assert(__builtin_offsetof(struct Padded, mi_last) == 17, "the next");
_Static_assert(__builtin_offsetof(struct Padded, mi_plain) == 18, "a zero");
_Static_assert(__builtin_offsetof(struct Padded, mi_one) == 24, "the first");
_Static_assert(__builtin_offsetof(struct Padded, mi_two) == 32, "the second");
_Static_assert(_Alignof(struct Padded) == 16, "a struct takes its strictest");
_Static_assert(sizeof(struct Padded) == 48, "a size is a multiple of it");
_Static_assert(_Alignof(union Either) == 4, "a union takes its strictest");
_Static_assert(sizeof(union Either) == 4, "and is padded to it");

char first = 1;
_Alignas(CACHE) char cached = 2;
char second = 3;
_Alignas(PAGE) static char paged[3];
_Alignas(double) _Alignas(WIDE) _Alignas(0) char strictest;
extern _Alignas(WIDE) char later;
_Alignas(WIDE) char later = 4;
int _Alignas(WORD) between = 5;

int on(const void *ptr, unsigned long align)
{
    return ((unsigned long) ptr & (align - 1)) == 0;
}

int main(void)
{
    char a = 1;
    _Alignas(WIDE) char b = 2;
    char c = 3;
    _Alignas(WORD) short d = 4;
    _Alignas(int) char e[3] = {5, 6, 7};
    _Alignas(CACHE) static char f = 8;
    _Alignas(0) char g = 9;
    struct Padded padded = {1, 2, 3, 4, 5, 6, 7};
    int sum = 0;

    sum += on(&cached, CACHE) + on(paged, PAGE) + on(&strictest, WIDE);
    sum += on(&later, WIDE) + on(&between, WORD) + on(&f, CACHE);
    sum += on(&b, WIDE) + on(&d, WORD) + on(e, sizeof(int));
    sum += on(&padded.mi_word, WORD) + on(&padded.mi_wide, WIDE);
    sum += first + cached + second + later + between + f;
    sum += a + b + c + d + e[0] + e[1] + e[2] + g;
    sum += padded.mi_first + padded.mi_word + padded.mi_wide + padded.mi_last;
    sum += padded.mi_plain + padded.mi_one + padded.mi_two;
    return sum - 57;
}
