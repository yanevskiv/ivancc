// (Test) Compiler error: [ERR_SEM_VOID_VALUE]
// Can't use the result of a `void` function as a value.

void g(void);

int main(void)
{
    int x = g();

    return x;
}
