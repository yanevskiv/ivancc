// (Test) Compiler error: [ERR_SEM_ADD_POINTERS]
// Can't add two pointers.

int main(void)
{
    int *p = 0;

    return (int) (p + p);
}
