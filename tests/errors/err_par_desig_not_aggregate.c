// (Test) Compiler error: [ERR_PAR_DESIG_NOT_AGGREGATE]
// A member designator in the initializer of an array.

int a[2] = { .x = 1 };

int main(void)
{
    return 0;
}
