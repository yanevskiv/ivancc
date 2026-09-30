// (Test) Compiler error: [ERR_PAR_ARRAY_LEN_NOT_POSITIVE]
// A negative inner dimension is refused, under a variable-length outer one.

int main()
{
    int n = 2;
    int grid[n][3 - 5];

    return sizeof(grid) != 0;
}
