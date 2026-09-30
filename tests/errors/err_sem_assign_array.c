// (Test) Compiler error: [ERR_SEM_ASSIGN_ARRAY]
// Can't assign to an array.

int a[2];
int b[2];

int main(void)
{
    a = b;
    return 0;
}
