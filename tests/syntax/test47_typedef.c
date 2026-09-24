// (Test) Return: 200
// typedef binds a name to a type, which the lexer has to know about: `Point`
// below is a type specifier everywhere after its declaration, and an ordinary
// identifier in the declaration that introduces it.

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

    if (area(&q) + n != 42) return 5;
    return 200;
}
