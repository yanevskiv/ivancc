// (Test) Compiler error: [ERR_SEM_INCDEC_CONST]
// Can't increment a `const` object.

const int c = 1;

void f(void)
{
    c++;
}
