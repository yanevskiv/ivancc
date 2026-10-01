// (Test) Compiler error: [ERR_SEM_ASSIGN_POINTEE_NOT_COMPATIBLE]
// An argument converts to its prototype's parameter as by assignment, so a
// pointer to long does not pass for a pointer to int.

int first(int *p) { return *p; }

int main()
{
    long x = 1;

    return first(&x);
}
