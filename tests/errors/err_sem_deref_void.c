// (Test) Compiler error: [ERR_SEM_DEREF_VOID]
// A `void *` dereferenced for its value.

int main(void)
{
    void *p = 0;
    int x = *p;

    return x;
}
