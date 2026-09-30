// (Test) Compiler error: [ERR_SEM_ARGS_TOO_FEW]
// Can't call a variadic function without its named arguments.

int g(int a, ...);

int main(void)
{
    return g();
}
