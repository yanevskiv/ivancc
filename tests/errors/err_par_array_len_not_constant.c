// (Test) Compiler error: [ERR_PAR_ARRAY_LEN_NOT_CONSTANT]
// A file-scope array whose length is a variable.

int n = 3;
int a[n];

int main(void)
{
    return 0;
}
