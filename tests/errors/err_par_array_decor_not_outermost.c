// (Test) Compiler error: [ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST]
// `static` on the inner array of a parameter rather than its outermost.

void f(int a[3][static 3]);

int main(void)
{
    return 0;
}
