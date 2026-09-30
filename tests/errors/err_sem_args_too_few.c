// (Test) Compiler error: [ERR_SEM_ARGS_TOO_FEW]
// A variadic function called without its one named argument.

int g(int a, ...);

int main(void)
{
    return g();
}
