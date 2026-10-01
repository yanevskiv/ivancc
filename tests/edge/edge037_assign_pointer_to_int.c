// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_POINTER]
// A pointer cannot be assigned to an integer without a cast.

int x;

int main()
{
    long v;

    v = &x;
    return v != 0;
}
