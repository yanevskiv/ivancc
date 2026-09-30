// (Test) Compiler error: [ERR_SEM_COND_MISMATCH]
// A conditional cannot mix a void operand with a value.

int main()
{
    int c = 1;

    return c ? (void) 0 : 1;
}
