// (Test) Compiler error: [ERR_GEN_INIT_NOT_CONSTANT]
// A file-scope object initialized by another object's value.

int x;
int y = x;

int main(void)
{
    return 0;
}
