// (Test) Compiler error: [ERR_SEM_SUB_POINTER_FROM_INT]
// Can't subtract a pointer from an integer.

int main(void)
{
    int *p = 0;

    return (int) (1 - p);
}
