## Tests

Aggregate layout and initializer folding can look correct while every access repeats the same wrong offset, so runtime output alone cannot validate them. The test suite covers structures, self-referential pointers, unions, enums, typedefs, flexible arrays, and designated initializers, checking sizes and distinct failure codes. These tests provide the current regression boundary, while the combined guide can split into separate files later.

### Add: `test43_struct`

A structure's layout is invisible at run time, since every member reads back whatever was written to it. `test43_struct` declares `struct Point` for access and assignment, and `struct Padded` for layout alone. The test on `p.y` after `q.y += 8` is what catches a copy that shared storage rather than duplicating it.

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
    if (sizeof(struct Padded) != 12) return 2;

    p.x = 20;
    p.y = 7;
    q = p;
    q.y += 8;

    if (p.y != 7) return 3;

    struct Point init = {1, 2};
    struct Point half = {4};
    if (init.x != 1 || init.y != 2) return 5;
    if (half.x != 4 || half.y != 0) return 6;

    pad.c = 1;
    pad.n = 5;
    pad.d = 2;
    if (pad.c != 1 || pad.n != 5 || pad.d != 2) return 4;

    return q.x + q.y + pad.n + pad.c + pad.d - 1;
}
```

### Add: `test44_struct_ptr`

A structure that points at its own type is ordinary C, and a list walk is what such a type exists for. `test44_struct_ptr` declares `struct Node` and walks three of them through `sum()`. Both `p->next->val` and `(*p).next->val` are written, since C defines the first as the second and a compiler may handle only one.

```c
struct Node {
    int          val;
    struct Node *next;
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

    if (sizeof(struct Node) != 16) return 1;
    if (p->val != 12) return 2;
    if (p->next->val != 20) return 3;

    p->val = 12;
    (*p).next->val = 20;

    return sum(&a);
}
```

### Add: `test45_union`

A union places every member at offset zero and takes the size of its widest. `test45_union` writes one member of `union Word` and reads another, and asserts the size of `union Mixed`. `union Mixed` must be 8 bytes from its pointer, since one sized from its first member would report 1 and pass every value check.

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
    if (sizeof(union Mixed) != 8) return 2;

    w.whole = 0;
    w.bytes[0] = 40;
    w.bytes[1] = 0;
    if (w.whole != 40) return 3;

    w.bytes[1] = 1;
    if (w.whole != 296) return 4;

    m.n = 2;
    if (m.c != 2) return 5;

    return w.bytes[0] + m.n;
}
```

### Add: `test46_enum`

An enumeration constant is a number the compiler folds, and it appears in two positions that different grammar rules read. `test46_enum` uses a constant as a `case` label and as an array length, alongside the counting rules. `sizeof(table)` is asserted as 84, which is `GONE` multiplied by the size of an `int`.

```c
enum Color { RED, GREEN, BLUE };
enum Status { OK = 10, BUSY, FAILED = 20, GONE };

int classify(enum Status s)
{
    switch (s) {
        case OK: {
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
    int table[GONE];

    if (RED != 0 || GREEN != 1 || BLUE != 2) return 1;
    if (OK != 10 || BUSY != 11) return 2;
    if (FAILED != 20 || GONE != 21) return 3;
    if (sizeof(table) != 84) return 4;
    if (classify(OK) != 1 || classify(FAILED) != 2 || classify(BUSY) != 0) return 5;

    return c + FAILED + OK + GONE - 11;
}
```

### Add: `test47_typedef`

A typedef binds a name to a type, and the lexer returns a different token once that binding exists. `test47_typedef` declares a typedef of a primitive, of a pointer, of a structure in the same statement, and of another typedef. The `String` initializer catches a typedef of a pointer that dropped its star.

```c
typedef int Int;
typedef char *String;

typedef struct Point {
    Int x;
    Int y;
} Point;

typedef Point Pair;

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

### Add: `test52_flex_array`

A flexible array member names the storage following a structure and contributes nothing to its size. `test52_flex_array` declares `struct Buf` and `struct Wide`, both of which must come out at 4 bytes. Both structures sit over a static array, since this stage has no allocator and the cast is what gives the member real storage.

```c
struct Buf {
    int  len;
    char data[];
};

struct Wide {
    char tag;
    int  vals[];
};

char store[64];
char other[64];

int main()
{
    struct Buf  *b = (struct Buf *) store;
    struct Wide *w = (struct Wide *) other;

    if (sizeof(struct Buf) != 4) return 1;
    if (sizeof(struct Wide) != 4) return 2;

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

### Add: `test53_designated_nested`

An initializer can nest, omit braces and aim an item at a subobject, and all three can appear in one. `test53_designated_nested` declares every form twice, once at file scope and once inside `main()`. `rec` leaves `c` and `p.x` unmentioned, and both must read back as zero, which only a clear before the stores can guarantee.

```c
struct Point { int x; int y; };
struct Line  { struct Point a; struct Point b; };
struct Rec   { int n; char c; struct Point p; };

struct Line  line   = {{1, 2}, {3, 4}};
struct Line  chain  = {.b.x = 7, .a.y = 3};
struct Rec   rec    = {.n = 5, .p = {.y = 9}};
int          grid[2][3] = {{1, 2, 3}, {4, 5, 6}};
int          flat[2][2] = {1, 2, 3, 4};
int          sparse[6]  = {[4] = 40, [1] = 10};

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
