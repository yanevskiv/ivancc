// Declarations and definitions test65_include reads through an include.

struct Pair { int a; int b; };
typedef struct Pair Pair;

int pair_count = 3;

int pair_sum(Pair p)
{
    return p.a + p.b;
}

int pair_scale(Pair *p, int by);
