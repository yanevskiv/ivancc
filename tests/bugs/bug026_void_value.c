// (Test) Compiler error: [ERR_SEM_VOID_VALUE]
// bug026. A void expression was used as a value, and the value read was
// whatever the register held.

void nothing(void) { }

int main()
{
    int x;

    x = nothing();
    return x;
}
