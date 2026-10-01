// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_COMPATIBLE]
// A static initializer converts as by assignment too, so a double cannot
// initialize a pointer.

int *p = 1.5;

int main()
{
    return p != 0;
}
