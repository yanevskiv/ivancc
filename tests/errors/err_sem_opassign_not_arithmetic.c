// (Test) Compiler error: [ERR_SEM_OPASSIGN_NOT_ARITHMETIC]
// Can't subtract with `-=` from a struct.

struct S { int a; } s;

void f(void)
{
    s -= 1;
}
