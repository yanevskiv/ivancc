// (Test) Compiler error: [ERR_SEM_RETURN_VALUE_IN_VOID]
// A function returning void cannot return a value.

void f(void)
{
    return 1;
}

int main()
{
    f();
    return 0;
}
