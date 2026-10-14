// (Test) Compiler error: [ERR_SEM_OPASSIGN_CONST]
// Can't assign with `op=` to a `const` object.

const int c = 1;

void f(void)
{
    c += 2;
}
