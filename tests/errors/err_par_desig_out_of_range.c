// (Test) Compiler error: [ERR_PAR_DESIG_OUT_OF_RANGE]
// An index designator one past the end of the array.

int a[3] = { [3] = 1 };

int main(void)
{
    return 0;
}
