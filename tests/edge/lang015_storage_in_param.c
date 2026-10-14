// (Test) Compiler error: [ERR_PAR_STORAGE_NOT_ALLOWED]
// A parameter takes no storage class but `register`.

int f(int static x)
{
    return x;
}

int main()
{
    return f(0);
}
