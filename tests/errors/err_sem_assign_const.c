// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// An assignment to a `const` object.

const int c = 1;

int main(void)
{
    c = 2;
    return c;
}
