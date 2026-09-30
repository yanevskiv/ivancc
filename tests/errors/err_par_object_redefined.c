// (Test) Compiler error: [ERR_PAR_OBJECT_REDEFINED]
// A file-scope object given two initializers.

int x = 1;
int x = 2;

int main(void)
{
    return 0;
}
