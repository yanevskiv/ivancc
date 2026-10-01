// (Test) Compiler error: [ERR_SEM_INCDEC_CONST]
// A const object cannot be incremented, postfix or prefix, or compound-assigned.

int main()
{
    const int c = 1;

    c++;
    return c;
}
