// (Test) Compiler error: [ERR_PAR_BODY_NOT_FUNCTION]
// A body after the declarator of an object.

int x
{
    return 0;
}

int main(void)
{
    return 0;
}
