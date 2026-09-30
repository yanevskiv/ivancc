// (Test) Compiler error: [ERR_SEM_ADD_POINTERS]
// Two pointers added together.

int main(void)
{
    int *p = 0;

    return (int) (p + p);
}
