// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_POINTER]
// A returned value converts to the return type as by assignment, so a pointer
// does not return as an int.

int x;

int address(void)
{
    return &x;
}

int main()
{
    return address() != 0;
}
