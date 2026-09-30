// (Test) Compiler error: [ERR_SEM_ARGS_WRONG_COUNT]
// Can't call a function with more arguments than parameters.

int g(int a);

int main(void)
{
    return g(1, 2);
}
