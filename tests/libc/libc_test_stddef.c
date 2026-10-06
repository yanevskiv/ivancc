// (Test) Status: 0
// <stddef.h> declares size_t, ptrdiff_t, wchar_t, NULL and offsetof, 7.17.

#include <stddef.h>

struct Pair {
    char c;
    int i;
    char tail[3];
};

int main(void)
{
    struct Pair p;
    char array[offsetof(struct Pair, tail)];
    ptrdiff_t diff = &p.tail[2] - &p.tail[0];
    wchar_t wide = L'a';

    if (sizeof(size_t) != sizeof(sizeof 0)) return 1;
    if ((size_t) -1 < 0) return 2;
    if ((ptrdiff_t) -1 > 0) return 3;
    if (diff != 2) return 4;
    if (wide != 97) return 5;
    if (NULL != 0) return 6;
    if ((void *) NULL != (void *) 0) return 7;
    if (offsetof(struct Pair, c) != 0) return 8;
    if (offsetof(struct Pair, i) != 4) return 9;
    if (offsetof(struct Pair, tail[1]) != 9) return 10;
    if (sizeof(array) != 8) return 11;
    return 0;
}
