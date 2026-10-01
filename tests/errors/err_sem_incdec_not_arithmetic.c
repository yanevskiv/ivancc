// (Test) Compiler error: [ERR_SEM_INCDEC_NOT_ARITHMETIC]
// Can't increment a struct.

struct S { int a; } s;

void f(void)
{
    s++;
}
