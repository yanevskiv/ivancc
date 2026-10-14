// (Test) Compiler error: [ERR_GEN_INIT_NOT_CONSTANT]
// The address of an automatic object is no constant, even offset into an array.

int main()
{
    int local[4];
    static int *p = &local[1];

    return *p;
}
