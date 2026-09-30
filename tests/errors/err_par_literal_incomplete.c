// (Test) Compiler error: [ERR_PAR_LITERAL_INCOMPLETE]
// Can't have a compound literal of an incomplete type.

struct S;

int main(void)
{
    (struct S){ 0 };
    return 0;
}
