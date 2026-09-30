// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// bug023. `const` was never enforced, so an object declared const could be
// assigned.

int main()
{
    int const x = 5;

    x = 6;
    return x;
}
