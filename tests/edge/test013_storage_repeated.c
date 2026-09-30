// (Test) Compiler error: [ERR_PAR_STORAGE_REPEATED]
// A declaration takes at most one storage class, wherever each is written.

int static const extern x;

int main()
{
    return 0;
}
