// (Test) Compiler error: [ERR_SEM_ARGS_WRONG_COUNT]
// A function of one parameter called with two arguments.

int g(int a);

int main(void)
{
    return g(1, 2);
}
