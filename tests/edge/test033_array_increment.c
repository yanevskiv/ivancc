// (Test) Compiler error: [ERR_SEM_INCDEC_CONST]
// An array is no modifiable lvalue, so `a++` is refused rather than producing
// a garbage value.

int main()
{
    int a[3] = { 1, 2, 3 };

    a++;
    return a[0];
}
