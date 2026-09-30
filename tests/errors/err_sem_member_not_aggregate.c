// (Test) Compiler error: [ERR_SEM_MEMBER_NOT_AGGREGATE]
// A member of an `int`.

int f(int n)
{
    return n.a;
}

int main(void)
{
    return 0;
}
