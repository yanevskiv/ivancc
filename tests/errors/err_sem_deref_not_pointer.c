// (Test) Compiler error: [ERR_SEM_DEREF_NOT_POINTER]
// Can't dereference an `int`.

int f(int n)
{
    return *n;
}
