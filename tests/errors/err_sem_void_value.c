// (Test) Compiler error: [ERR_SEM_VOID_VALUE]
// The result of a `void` function used as a value.

void g(void);

int main(void)
{
    int x = g();

    return x;
}
