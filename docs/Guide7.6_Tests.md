## Tests

Seven programs cover the stage, one per feature. Each returns 42 on success and a distinct small number at whichever check failed first. Every test that declares a type asserts on `sizeof`, because layout is the one place an error hides in plain sight.

### Structure declaration and access

The first program covers declaration, member access, and whole-structure assignment. `struct Point` is the simple shape and `struct Padded` is the one that checks layout. Both are declared at file scope and used inside `main()`.

`q = p` is checked in both directions. The members must arrive in `q`, and `p` must be left untouched afterwards. The test on `p.y` after `q.y += 8` is what catches a copy that shared storage instead of duplicating it.

`struct Padded` is chosen so that its size comes out wrong under any of the usual mistakes. A `char`, an `int`, and a `char` occupy 12 bytes. Forgetting interior padding gives 6, and forgetting tail padding gives 9.

`init` and `half` cover the two initializer cases a structure has. A complete list fills every member, while a short list leaves the rest at zero. The assertion on `half.y` is what catches a missing clear.

Expectations such as 12 are worth confirming against a real compiler before trusting them. The expected value is easier to get wrong than the compiler under test. A wrong expectation is the one failure that teaches nothing.

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

The second program covers `->`, self-reference, and a walk over a list. `struct Node` holds an `int` and a pointer to its own type. `sum()` walks from a head pointer to the end of the chain.

The declaration of `next` proves the tag was bound before the member list was read. An implementation that binds the tag at the closing brace fails here outright. Nothing else in the file gets a chance to run.

`sizeof(struct Node)` catches a layout pass that ignores alignment. Four bytes of `int` followed by an eight-byte pointer is 16 bytes rather than 12. Only the interior padding accounts for the difference.

`p->next->val` and `(*p).next->val` are both written deliberately. C defines the first as the second, so the two must take identical paths through the compiler. A grammar that builds `->` as its own node kind rather than as a dereference fails one of them.

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

The third program covers overlapping members and union sizing. `union Word` overlays an `int` on four `char`s. `union Mixed` exists only to check that a union takes its widest member's size.

Writing one member and reading another is the only way the overlap becomes visible. `w.bytes[0] = 40` followed by a read of `w.whole` is that check. A union giving its members distinct offsets would pass every other assertion in the file.

The second write puts 1 into the byte above the low one. `w.whole` then reads 296, which is 40 plus 256. The value depends on the target being little-endian, which this compiler assumes throughout.

`union Mixed` holds a `char`, an `int`, and a `char *`. Its size must be 8, taken from the pointer. A union sized from its first member would report 1 and still pass every value check in the program.

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

The fourth program covers counting, explicit values, and the two positions where a constant is read. `enum Color` checks the default count from zero. `enum Status` checks what an explicit value does to that count.

`RED`, `GREEN`, and `BLUE` must come out as 0, 1, and 2. `OK = 10` then makes `BUSY` 11 rather than 1. `FAILED = 20` resets the count again, which makes `GONE` 21.

`int table[GONE];` is the check that fails when constants fold only inside expressions. Folding in expressions is the easy half of the job to implement and then forget. Nothing in a normal program notices until an array is declared this way.

The `case` labels in `classify()` are the other half. A label is read by a different rule from an expression in most grammars. A constant that folds in one may well not fold in the other.

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

The fifth program covers four typedef shapes. `Int` names a primitive and `String` names a pointer. `Point` names a structure declared in the same statement, and `Pair` names another typedef.

`Point` is the shape that exercises the lexer feedback. It is an ordinary identifier on the line that introduces it and a type specifier on every line after. That is the whole ordering constraint, expressed in a file that either compiles or does not.

`typedef Point Pair;` requires the feedback to have taken effect already. `Point` must arrive as `TYPEDEF_NAME` for that declaration to reduce at all. `area()` then takes a `Pair *` and reaches members declared as `Int`.

The `String` declaration checks that a typedef of a pointer keeps the pointer. A compiler that binds `String` to `char` and drops the star fails on the initializer. The check on `s[0]` confirms the subscript still works afterwards.

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

The sixth program covers a trailing member declared with empty brackets. `struct Buf` puts a `char` array after an `int`. `struct Wide` puts an `int` array after a `char`.

Both structures are 4 bytes. The flexible member names the storage following the structure without occupying any of its own. A layout pass that gave it even one element would push both sizes up.

`struct Wide` is the more interesting of the two. Its member contributes no size and still raises the alignment of the whole structure to 4. The single `char` before it is therefore padded out to four bytes.

Both structures are placed over a static array rather than allocated, since this stage has no allocator. The cast is what gives the trailing member real storage to address. Writing past `sizeof` is the entire point of the construct.

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

The seventh program covers every initializer form the flattener handles. The same declarations appear twice, once at file scope and once inside `main()`. A global writes bytes into an image, while a local emits stores.

`line` and `grid` are ordinary nested lists. `flat` is brace elision, and `chain` is a designator walking two levels down. `rec` mixes a designator with a nested list and leaves gaps behind it.

`sparse` writes its items out of order, with `[4]` ahead of `[1]`. A walk that only ever advances a cursor would get that wrong. `sparse[0]` is checked as well, since the items between the two designators must stay zero.

`rec` is the check on zeroing. `.n` and `.p.y` are written, while `c` and `p.x` are never mentioned. Both must read back as zero, which only a full clear before the stores can guarantee.

The file-scope declarations and the locals are deliberately near-duplicates. A flattener shared between the two paths passes both sets of checks. One that quietly differs between them fails exactly half of them.

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
