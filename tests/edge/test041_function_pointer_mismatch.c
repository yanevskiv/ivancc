// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_COMPATIBLE]
// Pointers to functions with different parameter lists are incompatible.

int none(void) { return 0; }

int main()
{
    int (*f)(int) = none;

    return f(1);
}
