# Aggregates

This stage implements `struct`, `union`, `enum`, and `typedef` by reference rather than by value. That covers declaring them, laying their members out as the SysV ABI does, reaching a member with `.` and `->`, assigning one whole, and initializing them with braces and designators.

Passing or returning an aggregate by value is out of scope here. It requires the ABI's argument classification algorithm, which belongs to a later stage, so the semantic pass rejects those cases rather than compiling them wrongly.

The stage is split into six parts, in the order the toolchain is built: [Lexer](Guide7.1_Lexer.md), [Parser](Guide7.2_Parser.md), [Ast](Guide7.3_Ast.md), [Sem](Guide7.4_Sem.md), [Gen](Guide7.5_Gen.md), and [Tests](Guide7.6_Tests.md). Each part stands on its own, and later phases of this stage extend the same six files rather than adding new ones.

The program below is the whole target. A compiler that already handles scalars, pointers, and arrays compiles it once every part is in place.

```c
struct Point { int x; int y; };

typedef struct Line {
    struct Point a;
    struct Point b;
} Line;

enum Side { LEFT, RIGHT };

union Word {
    int  whole;
    char bytes[4];
};

int main()
{
    Line l = {{1, 2}, .b.y = 9};
    struct Point *p = &l.a;

    p->x = LEFT;
    return l.b.y + p->x;
}
```