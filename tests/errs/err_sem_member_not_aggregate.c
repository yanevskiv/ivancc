// (Test) Compiler error: [ERR_SEM_MEMBER_NOT_AGGREGATE]
// Can't access a member of an `int`.

int f(int n)
{
    return n.a;
}
