// (Test) Compiler error: [ERR_SEM_RETURN_VALUE_IN_VOID]
// A `void` function returning a value.

void f(void)
{
    return 1;
}

int main(void)
{
    return 0;
}
