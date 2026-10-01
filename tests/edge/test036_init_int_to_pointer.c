// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_POINTER]
// An integer other than a null pointer constant cannot initialize a pointer.

int main()
{
    int *p = 5;

    return p != 0;
}
