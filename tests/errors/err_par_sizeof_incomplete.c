// (Test) Compiler error: [ERR_PAR_SIZEOF_INCOMPLETE]
// `sizeof` of a struct declared but never defined.

struct S;
unsigned long n = sizeof(struct S);

int main(void)
{
    return 0;
}
