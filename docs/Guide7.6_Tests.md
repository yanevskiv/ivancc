## Tests

Seven programs cover the stage, one per feature. Each returns 42 on success and a distinct small number at whichever check failed first, so a failing run names the assertion rather than merely reporting a mismatch. Every test that declares a type also asserts on `sizeof`.

### Structure declaration and access

The first program covers declaration, member access, and whole-structure assignment. `struct Point` is the simple shape, with two `int` members and no padding, while `struct Padded` exists only to check layout.

`struct Padded` is chosen so that its size comes out wrong under any of the usual mistakes. Forgetting interior padding gives 6 and forgetting tail padding gives 9, rather than the correct 12.

`q = p` is checked in both directions. The test on `p.y` after `q.y += 8` catches a generator that copied the address rather than the bytes, which the first check alone would miss.

`init` and `half` cover the two initializer cases a structure has. The assertion on `half.y` catches a missing clear, since that member would otherwise hold whatever the stack slot contained.

```c
struct Point {
    int x;
    int y;
};

struct Padded {
    char c;
    int  n;
    char d;
};

int main()
{
    struct Point p;
    struct Point q;
    struct Padded pad;

    if (sizeof(struct Point) != 8) return 1;
    if (sizeof(struct Padded) != 12) return 2;  /* not 6, and not 9 */

    p.x = 20;
    p.y = 7;
    q = p;
    q.y += 8;

    if (p.y != 7) return 3;                     /* the copy must be independent */

    struct Point init = {1, 2};
    struct Point half = {4};
    if (init.x != 1 || init.y != 2) return 5;
    if (half.x != 4 || half.y != 0) return 6;   /* the omitted member is zero */

    pad.c = 1;
    pad.n = 5;
    pad.d = 2;
    if (pad.c != 1 || pad.n != 5 || pad.d != 2) return 4;

    return q.x + q.y + pad.n + pad.c + pad.d - 1;
}
```

### Structure pointers

The second program covers `->`, a structure that points at its own type, and a walk over a linked list. `sum()` uses `->` in both a loop condition and an expression, which a single access would not exercise.

The declaration of `next` proves the tag was bound before the member list was read. An implementation that binds it at the closing brace fails there outright, before anything else in the file can run.

`sizeof(struct Node)` catches a layout pass that ignores alignment. Four bytes of `int` followed by an eight-byte pointer is 16 rather than 12, and no value check would expose the difference.

`p->next->val` and `(*p).next->val` are both written deliberately. C defines the first form as the second, and a grammar that builds `->` as its own node kind usually handles only one of them.

```c
struct Node {
    int          val;
    struct Node *next;  /* requires the tag to be bound already */
};

int sum(struct Node *head)
{
    int total = 0;

    for (struct Node *n = head; n; n = n->next) {
        total += n->val;
    }
    return total;
}

int main()
{
    struct Node a;
    struct Node b;
    struct Node c;
    struct Node *p = &a;

    a.val = 12; a.next = &b;
    b.val = 20; b.next = &c;
    c.val = 10; c.next = 0;

    if (sizeof(struct Node) != 16) return 1;    /* 4 bytes padded to 8, then 8 */
    if (p->val != 12) return 2;
    if (p->next->val != 20) return 3;

    p->val = 12;
    (*p).next->val = 20;                        /* `->` and `(*p).` agree */

    return sum(&a);
}
```

### Union member overlap

The third program covers overlapping members and union sizing. `union Word` overlays an `int` on four `char`s, while `union Mixed` exists only to check that a union takes its widest member's size.

Writing one member and reading another is the only way the overlap becomes visible, so `w.bytes[0] = 40` is followed by a read of `w.whole`. A union giving its members distinct offsets would pass every other assertion.

The second write puts 1 into the byte above the low one, so `w.whole` reads 296. That expectation assumes a little-endian target, which this compiler assumes throughout.

`union Mixed` must be 8 bytes, taken from its pointer member. A union sized from its first member instead would report 1 and still pass every value check in the program.

```c
union Word {
    int  whole;
    char bytes[4];
};

union Mixed {
    char  c;
    int   n;
    char *p;
};

int main()
{
    union Word w;
    union Mixed m;

    if (sizeof(union Word) != 4) return 1;
    if (sizeof(union Mixed) != 8) return 2;     /* the pointer is widest */

    w.whole = 0;
    w.bytes[0] = 40;
    w.bytes[1] = 0;
    if (w.whole != 40) return 3;

    w.bytes[1] = 1;
    if (w.whole != 296) return 4;               /* 40 + 256 */

    m.n = 2;
    if (m.c != 2) return 5;

    return w.bytes[0] + m.n;
}
```

### Enumeration constants

The fourth program covers counting from zero, explicit values, and the two positions where a constant is read. Those positions are a `case` label and an array length, which different grammar rules usually handle.

`OK = 10` makes `BUSY` 11 rather than 1, which proves the counter was reset rather than restarted. `FAILED = 20` resets it again, so `GONE` must come out as 21.

`int table[GONE];` is the check that fails when constants fold only inside expressions. Folding there is the easy half of the job, and nothing notices the other half until an array is declared this way.

The `case` labels in `classify()` are that other half. `classify(BUSY)` returning 0 checks the default path, since a `switch` matching every label would pass the two positive cases unnoticed.

```c
enum Color { RED, GREEN, BLUE };
enum Status { OK = 10, BUSY, FAILED = 20, GONE };

int classify(enum Status s)
{
    switch (s) {
        case OK: {                              /* a constant as a case label */
            return 1;
        } break;
        case FAILED: {
            return 2;
        } break;
    }
    return 0;
}

int main()
{
    enum Color c = BLUE;
    int table[GONE];                            /* a constant as an array length */

    if (RED != 0 || GREEN != 1 || BLUE != 2) return 1;
    if (OK != 10 || BUSY != 11) return 2;       /* an explicit value moves the count */
    if (FAILED != 20 || GONE != 21) return 3;
    if (sizeof(table) != 84) return 4;          /* 21 * 4 */
    if (classify(OK) != 1 || classify(FAILED) != 2 || classify(BUSY) != 0) return 5;

    return c + FAILED + OK + GONE - 11;
}
```

### Typedef declarations

The fifth program covers four typedef shapes: a primitive, a pointer, a structure declared in the same statement, and another typedef. Together they cover every form the declarator rules carry into a bound type.

`Point` is the shape that exercises the lexer feedback. It is an ordinary identifier on the line that introduces it and a type specifier on every line after, which no assertion can test directly.

`typedef Point Pair;` requires that feedback to have taken effect already. `area()` then reaches members declared as `Int` through a `Pair *`, which chains three typedefs in one expression.

The `String` declaration checks that a typedef of a pointer keeps the pointer. A compiler that binds `String` to `char` fails on the initializer rather than somewhere less obvious later.

```c
typedef int Int;
typedef char *String;

typedef struct Point {
    Int x;
    Int y;
} Point;                                        /* `Point` is an IDENT here */

typedef Point Pair;                             /* and a TYPEDEF_NAME here */

int area(Pair *p)
{
    return p->x * p->y;
}

int main()
{
    Point  p;
    Pair   q;
    Int    n = 2;
    String s = "ok";

    if (sizeof(Point) != 8) return 1;
    if (sizeof(Int) != 4) return 2;

    p.x = 5;
    p.y = 8;
    q = p;

    if (area(&q) != 40) return 3;
    if (s[0] != 'o') return 4;

    return area(&q) + n;
}
```

### Flexible array members

The sixth program covers a trailing member declared with empty brackets. `struct Buf` puts a `char` array after an `int`, and `struct Wide` puts an `int` array after a `char`.

Both structures are 4 bytes, since the flexible member names storage after the structure without occupying any of its own. A layout pass that gave it even one element would push both sizes up.

`struct Wide` is the more interesting of the two. Its member contributes no size and still raises the alignment to 4, so an implementation that skipped it entirely would report 1.

Both structures are placed over a static array, since this stage has no allocator. The cast is what gives the trailing member real storage, and `store` is large enough to hold what is written.

```c
struct Buf {
    int  len;
    char data[];   /* no size, no alignment change */
};

struct Wide {
    char tag;
    int  vals[];   /* no size, but alignment 4 */
};

char store[64];
char other[64];

int main()
{
    struct Buf  *b = (struct Buf *) store;
    struct Wide *w = (struct Wide *) other;

    if (sizeof(struct Buf) != 4) return 1;
    if (sizeof(struct Wide) != 4) return 2;     /* 1 byte padded to 4 */

    b->len = 3;
    b->data[0] = 'a';
    b->data[1] = 'b';
    b->data[2] = 'c';
    if (b->data[0] != 'a' || b->data[2] != 'c') return 3;

    w->tag = 1;
    w->vals[0] = 30;
    w->vals[1] = 9;
    if (w->tag != 1) return 4;

    return b->len + w->vals[0] + w->vals[1];
}
```

### Nested and designated initializers

The seventh program covers every initializer form the flattener handles. The same declarations appear at file scope and again inside `main()`, since a global writes an image while a local emits stores.

`flat` is brace elision and `chain` is a designator walking two levels down. `rec` mixes a designator with a nested list, which is the case that combines two features in one initializer.

`sparse` writes its items out of order, so a walk that only ever advances a cursor would get it wrong. `sparse[0]` is checked too, because the elements around the designators must stay zero.

`rec` is the check on zeroing. `c` and `p.x` are never mentioned in the initializer yet must read back as zero, which only a full clear before the stores can guarantee.

The file-scope declarations and the locals are deliberately near-duplicates. A flattener that differs between the two paths fails exactly half the checks, which points straight at the path that diverged.

```c
struct Point { int x; int y; };
struct Line  { struct Point a; struct Point b; };
struct Rec   { int n; char c; struct Point p; };

struct Line  line   = {{1, 2}, {3, 4}};
struct Line  chain  = {.b.x = 7, .a.y = 3};     /* two levels down */
struct Rec   rec    = {.n = 5, .p = {.y = 9}};  /* `c` and `p.x` stay zero */
int          grid[2][3] = {{1, 2, 3}, {4, 5, 6}};
int          flat[2][2] = {1, 2, 3, 4};         /* brace elision */
int          sparse[6]  = {[4] = 40, [1] = 10}; /* out of order */

int main()
{
    struct Line  l = {{1, 2}, {3, 4}};
    struct Rec   r = {.c = 'z', .n = 3};
    int          m[2][2] = {{1, 2}, {3, 4}};
    int          e[2][2] = {1, 2, 3, 4};

    if (line.a.x != 1 || line.b.y != 4) return 1;
    if (chain.b.x != 7 || chain.a.y != 3 || chain.a.x != 0) return 2;
    if (rec.n != 5 || rec.p.y != 9 || rec.p.x != 0 || rec.c != 0) return 3;
    if (grid[0][2] != 3 || grid[1][0] != 4) return 4;
    if (flat[0][1] != 2 || flat[1][1] != 4) return 5;
    if (sparse[1] != 10 || sparse[4] != 40 || sparse[0] != 0) return 6;

    if (l.a.x != 1 || l.b.y != 4) return 7;
    if (r.c != 'z' || r.n != 3 || r.p.x != 0) return 8;
    if (m[0][1] != 2 || m[1][0] != 3) return 9;
    if (e[0][1] != 2 || e[1][1] != 4) return 10;

    return sparse[4] + r.n - 1;
}
```
