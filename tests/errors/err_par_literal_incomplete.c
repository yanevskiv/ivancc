// (Test) Compiler error: [ERR_PAR_LITERAL_INCOMPLETE]
// A compound literal of a struct declared but never defined.

struct S;

int main(void)
{
    (struct S){ 0 };
    return 0;
}
