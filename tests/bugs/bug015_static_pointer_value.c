// (Test) Compiler error: [ERR_GEN_INIT_NOT_CONSTANT]
// bug015. A static pointer initialized from another pointer variable took that
// variable's address, where the value of an object is no constant.

int x;
int *gp = &x;
int *q = gp;

int main()
{
    return q == gp;
}
