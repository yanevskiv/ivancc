// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// An object reached through a pointer to const is read-only.

int x;

int main()
{
    const int *p = &x;

    *p = 1;
    return 0;
}
