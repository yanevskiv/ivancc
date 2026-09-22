# Aggregates

Aggregates need named members, tags, and layout rules that scalar types do not, but the compiler had no representation for them. The test exercises `struct`, `union`, `enum`, and `typedef` through access, assignment, pointers, and initialization, so each aggregate path must work. This combined guide covers four tests through the [Lexer](Guide7.1_Lexer.md), [Parser](Guide7.2_Parser.md), [Ast](Guide7.3_Ast.md), [Sem](Guide7.4_Sem.md), [Gen](Guide7.5_Gen.md), and [Tests](Guide7.6_Tests.md) parts; later stages will split it.

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
