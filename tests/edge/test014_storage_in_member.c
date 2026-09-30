// (Test) Compiler error: [ERR_PAR_STORAGE_NOT_ALLOWED]
// A struct member takes no storage class, even after its type.

struct S {
    int static a;
};

int main()
{
    return 0;
}
