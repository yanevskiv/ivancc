// (Test) Status: 200
// __builtin_offsetof, as gcc's. It yields a size_t, the offset of a member
// reached through members and indices, `a->b` standing for `a[0].b`. With
// constant indices it is an integer constant, good for an array length, a
// static initializer, a case label or an enumerator. An index known only at
// run time is evaluated once, and the offset is computed where it runs.

struct In {
    char c;
    int  v[3];
};

struct S {
    char      a;
    double    d;
    short     s;
    long      arr[4];
    struct In in[2];
    union {
        char  u1;
        long  u2;
    } u;
    int       bits : 3;
    int       tail;
    char      flex[];
};

typedef struct S S_t;

union U {
    char  c;
    int   i;
    long  l[2];
};

// Constants at file scope: an array length and static initializers.
static char sized[__builtin_offsetof(struct S, s)];
static unsigned long offs[] = {
    __builtin_offsetof(struct S, a),
    __builtin_offsetof(struct S, arr[3]),
    __builtin_offsetof(struct S, in[1].v[2])
};

enum {
    OFF_D   = __builtin_offsetof(struct S, d),
    OFF_ARR = __builtin_offsetof(struct S, arr)
};

// The classic macro's offset, computed at run time for comparison.
#define CLASSIC(T, m) ((unsigned long) &((T *) 0)->m)

static int calls;

int count(int n)
{
    calls++;
    return n;
}

int classify(unsigned long off)
{
    switch (off) {
        case __builtin_offsetof(struct S, d): return 1;
        case __builtin_offsetof(struct S, s): return 2;
        case __builtin_offsetof(struct S, in): return 3;
        default: return 0;
    }
}

int main(void)
{
    // Members, padding included.
    if (__builtin_offsetof(struct S, a) != 0) return 1;
    if (__builtin_offsetof(struct S, d) != 8) return 2;
    if (__builtin_offsetof(struct S, s) != 16) return 3;
    if (__builtin_offsetof(struct S, arr) != 24) return 4;
    if (__builtin_offsetof(struct S, in) != 56) return 5;
    if (__builtin_offsetof(struct S, u) != 88) return 6;
    if (__builtin_offsetof(struct S, tail) != 100) return 7;
    if (__builtin_offsetof(struct S, flex) != 104) return 8;

    // Indices and nested members.
    if (__builtin_offsetof(struct S, arr[2]) != 40) return 9;
    if (__builtin_offsetof(struct S, in[1]) != 72) return 10;
    if (__builtin_offsetof(struct S, in[1].v) != 76) return 11;
    if (__builtin_offsetof(struct S, in[1].v[2]) != 84) return 12;
    if (__builtin_offsetof(struct S, u.u2) != 88) return 13;
    if (__builtin_offsetof(struct S, flex[5]) != 109) return 14;
    if (__builtin_offsetof(struct S, arr[1 + 1]) != CLASSIC(struct S, arr[2])) return 15;

    // `a->b` is `a[0].b`.
    if (__builtin_offsetof(struct S, in->v) != 60) return 16;
    if (__builtin_offsetof(struct S, in->v[1]) != 64) return 17;

    // Unions, typedefs and qualified types.
    if (__builtin_offsetof(union U, i) != 0 || __builtin_offsetof(union U, l[1]) != 8) return 18;
    if (__builtin_offsetof(S_t, d) != 8) return 19;
    if (__builtin_offsetof(const struct S, s) != 16) return 20;
    if (__builtin_offsetof(volatile S_t, in[0].c) != 56) return 21;

    // A size_t: unsigned and eight bytes wide.
    if (sizeof __builtin_offsetof(struct S, a) != 8) return 22;
    if (__builtin_offsetof(struct S, a) - 1 < 1) return 23;

    // Constants where C asks for them.
    if (sizeof sized != 16) return 24;
    if (offs[0] != 0 || offs[1] != 48 || offs[2] != 84) return 25;
    if (OFF_D != 8 || OFF_ARR != 24) return 26;
    if (classify(8) != 1 || classify(16) != 2 || classify(56) != 3 || classify(0) != 0) return 27;
    char local[__builtin_offsetof(struct In, v[1])];
    if (sizeof local != 8) return 28;

    // Indices known only at run time, each evaluated once.
    int i = 1;
    int j = 2;
    if (__builtin_offsetof(struct S, arr[i]) != 32) return 29;
    if (__builtin_offsetof(struct S, in[i].v[j]) != 84) return 30;
    if (__builtin_offsetof(struct S, arr[count(3)]) != 48 || calls != 1) return 31;
    if (__builtin_offsetof(struct S, arr[i++]) != 32 || i != 2) return 32;
    if (__builtin_offsetof(struct S, arr[0, 1]) != 32) return 33;
    if (sizeof __builtin_offsetof(struct S, arr[i]) != 8) return 34;
    for (int k = 0; k < 4; k++) {
        if (__builtin_offsetof(struct S, arr[k]) != CLASSIC(struct S, arr[k])) return 35;
    }

    // A length known only at run time makes a variable-length array.
    char vla[__builtin_offsetof(struct S, arr[j])];
    if (sizeof vla != 40) return 36;
    return 200;
}
