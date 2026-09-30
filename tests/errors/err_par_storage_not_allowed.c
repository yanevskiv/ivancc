// (Test) Compiler error: [ERR_PAR_STORAGE_NOT_ALLOWED]
// A storage class on a struct member.

struct S { static int a; };

int main(void)
{
    return 0;
}
