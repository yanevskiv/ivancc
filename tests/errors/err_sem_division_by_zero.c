// (Test) Compiler error: [ERR_SEM_DIVISION_BY_ZERO]
// A division by zero in an enumerator's value.

enum { A = 1 / 0 };

int main(void)
{
    return 0;
}
