// (Test) Compiler error: [ERR_SEM_OPERAND_NOT_INTEGER]
// `~` takes only integers, not a pointer.

int x;

int main()
{
    int *p = &x;

    return ~p != 0;
}
